#include <gtest/gtest.h>
#include <cstring>
#include "lib/drivers/gps/nmea.h"

TEST(NMEA, check_sentence_accepts_valid_checksum_and_line_ending)
{
    char sentence[] = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";

    EXPECT_TRUE(nmea_check_sentence(sentence));
}

TEST(NMEA, check_sentence_rejects_invalid_checksum)
{
    char sentence[] = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*00\r\n";

    EXPECT_FALSE(nmea_check_sentence(sentence));
}

TEST(NMEA, gets_sentence_and_talker_ids)
{
    char sentence[] = "$GPRMC,123519.00,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,A,A*44\r\n";

    EXPECT_EQ(nmea_get_sentence_id(sentence), NMEA_SENTENCE_RMC);
    EXPECT_EQ(nmea_get_talker_id(sentence), NMEA_TALKER_GPS_SBAS);
}

TEST(NMEA, parse_rmc_extracts_time_date_and_coordinates)
{
    char sentence[] = "$GPRMC,123519.00,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,A,A*44\r\n";
    nmea_sentence_rmc_t rmc;

    ASSERT_TRUE(nmea_parse_rmc(sentence, &rmc));

    EXPECT_NEAR(rmc.lat, 48.1173, 1e-4f);
    EXPECT_NEAR(rmc.lon, 11.5167, 1e-4f);
    EXPECT_EQ(rmc.NS, 'N');
    EXPECT_EQ(rmc.EW, 'E');
    EXPECT_EQ(rmc.time.hour, 12);
    EXPECT_EQ(rmc.time.minute, 35);
    EXPECT_EQ(rmc.time.second, 19);
    EXPECT_EQ(rmc.time.hundredth, 0);
    EXPECT_EQ(rmc.date.day, 23);
    EXPECT_EQ(rmc.date.month, 3);
    EXPECT_EQ(rmc.date.year, 94);
}

TEST(NMEA, parse_gsv_parses_satellite_entries)
{
    char sentence[] = "$GNGSV,2,1,08,01,40,083,41,02,17,308,43,12,22,228,42,14,13,172,42,1*7A\r\n";
    nmea_sentence_gsv_t gsv;

    ASSERT_TRUE(nmea_parse_gsv(sentence, &gsv));

    EXPECT_EQ(gsv.numMsg, 2);
    EXPECT_EQ(gsv.msgNum, 1);
    EXPECT_EQ(gsv.numSV, 8);
    EXPECT_EQ(gsv.sats[0].svid, 1);
    EXPECT_EQ(gsv.sats[0].elv, 40);
    EXPECT_EQ(gsv.sats[0].az, 83);
    EXPECT_EQ(gsv.sats[0].cno, 41);
    EXPECT_EQ(gsv.signalId, 1);
}

// --- Regression tests for the 's' format overflow fix ---

TEST(NMEA, scan_string_field_truncates_to_destination_size_without_overflow)
{
    // Struct layout guarantees `guard` sits immediately after `buf` with no padding,
    // since both are char arrays (alignment 1). Any overflow of the copy into `buf`
    // corrupts `guard`.
    struct Canary
    {
        char buf[6];
        unsigned char guard[4];
    };

    Canary c;
    memset(c.buf, 0x55, sizeof(c.buf));
    c.guard[0] = 0xAA;
    c.guard[1] = 0xBB;
    c.guard[2] = 0xCC;
    c.guard[3] = 0xDD;

    // First field is 26 characters, far longer than the 6-byte destination.
    char sentence[] = "$ABCDEFGHIJKLMNOPQRSTUVWXYZ,1,2*00\r\n";

    ASSERT_TRUE(nmea_scan(sentence, "s", c.buf, sizeof(c.buf)));

    EXPECT_STREQ(c.buf, "ABCDE");
    EXPECT_EQ(c.guard[0], 0xAA);
    EXPECT_EQ(c.guard[1], 0xBB);
    EXPECT_EQ(c.guard[2], 0xCC);
    EXPECT_EQ(c.guard[3], 0xDD);
}

TEST(NMEA, get_sentence_id_and_talker_id_survive_overlong_first_field)
{
    char sentence[] = "$ABCDEFGHIJKLMNOPQRSTUVWXYZ,1,2*00\r\n";

    // Must not crash / corrupt the stack; content won't match a known ID so both
    // resolve to UNKNOWN.
    EXPECT_EQ(nmea_get_sentence_id(sentence), NMEA_SENTENCE_UNKNOWN);
    EXPECT_EQ(nmea_get_talker_id(sentence), NMEA_TALKER_UNKNOWN);
}

TEST(NMEA, parse_gns_truncates_overlong_pos_mode_field)
{
    // posMode is char[4] (3 chars + null). Feed a pathologically long field.
    char sentence[] = "$GNGNS,012345.00,1234.5678,N,01234.5678,E,AAAAAAAAAAAAAAAAAAAA,08,1.0,100.0,10.0,,,A*00\r\n";
    nmea_sentence_gns_t gns;

    ASSERT_TRUE(nmea_parse_gns(sentence, &gns));

    EXPECT_STREQ(gns.posMode, "AAA");
}

TEST(NMEA, parse_gns_truncates_realistic_four_character_pos_mode_field)
{
    // A legitimate 4-constellation posMode ("AAAA") still doesn't fit char[4]
    // (needs a null terminator too); confirm it truncates safely rather than
    // overflowing.
    char sentence[] = "$GNGNS,012345.00,1234.5678,N,01234.5678,E,AAAA,08,1.0,100.0,10.0,,,A*00\r\n";
    nmea_sentence_gns_t gns;

    ASSERT_TRUE(nmea_parse_gns(sentence, &gns));

    EXPECT_STREQ(gns.posMode, "AAA");
}

// --- Regression tests for the missing-NUL-check fix ---

TEST(NMEA, parse_rmc_rejects_truncated_sentence_with_no_delimiter)
{
    // No comma or '*' anywhere after '$' before the string ends.
    char sentence[] = "$GPRMC";
    nmea_sentence_rmc_t rmc;

    EXPECT_FALSE(nmea_parse_rmc(sentence, &rmc));
}

TEST(NMEA, parse_gga_rejects_truncated_sentence_with_no_delimiter)
{
    char sentence[] = "$GPGGA";
    nmea_sentence_gga_t gga;

    EXPECT_FALSE(nmea_parse_gga(sentence, &gga));
}

TEST(NMEA, parse_gga_rejects_sentence_truncated_mid_field)
{
    // Cut off mid-transmission: six fields present, then the string just ends
    // with no trailing comma or '*' -- must not read past the buffer.
    char sentence[] = "$GPGGA,123519,4807.038,N,01131.000,E";
    nmea_sentence_gga_t gga;

    EXPECT_FALSE(nmea_parse_gga(sentence, &gga));
}

TEST(NMEA, scan_and_parsers_reject_empty_string)
{
    char sentence[] = "";
    nmea_sentence_rmc_t rmc;
    nmea_sentence_gga_t gga;

    EXPECT_FALSE(nmea_parse_rmc(sentence, &rmc));
    EXPECT_FALSE(nmea_parse_gga(sentence, &gga));
    EXPECT_EQ(nmea_get_sentence_id(sentence), NMEA_SENTENCE_UNKNOWN);
    EXPECT_EQ(nmea_get_talker_id(sentence), NMEA_TALKER_UNKNOWN);
}

TEST(NMEA, parse_rmc_rejects_dollar_only_sentence)
{
    char sentence[] = "$";
    nmea_sentence_rmc_t rmc;

    EXPECT_FALSE(nmea_parse_rmc(sentence, &rmc));
}

// --- nmea_check_sentence boundary/length tests ---

TEST(NMEA, check_sentence_handles_short_and_malformed_inputs)
{
    char empty[] = "";
    char dollarOnly[] = "$";
    char dollarStar[] = "$*";
    char noAsterisk[] = "$GPRMC,1,2,3\r\n";
    char asteriskAtEnd[] = "$GPRMC,1,2*";
    char oneChecksumDigit[] = "$GPRMC,1,2*4";

    EXPECT_FALSE(nmea_check_sentence(empty));
    EXPECT_FALSE(nmea_check_sentence(dollarOnly));
    EXPECT_FALSE(nmea_check_sentence(dollarStar));
    EXPECT_FALSE(nmea_check_sentence(noAsterisk));
    EXPECT_FALSE(nmea_check_sentence(asteriskAtEnd));
    EXPECT_FALSE(nmea_check_sentence(oneChecksumDigit));
}

// --- nmea_parse_gsv repetition-count boundary tests ---

TEST(NMEA, parse_gsv_handles_zero_satellite_entries)
{
    char sentence[] = "$GNGSV,1,1,00,1*7A\r\n";
    nmea_sentence_gsv_t gsv;

    ASSERT_TRUE(nmea_parse_gsv(sentence, &gsv));

    EXPECT_EQ(gsv.numMsg, 1);
    EXPECT_EQ(gsv.msgNum, 1);
    EXPECT_EQ(gsv.numSV, 0);
}

TEST(NMEA, parse_gsv_handles_four_satellite_entries)
{
    char sentence[] = "$GNGSV,1,1,16,01,10,010,11,02,20,020,22,03,30,030,33,04,40,040,44,1*79\r\n";
    nmea_sentence_gsv_t gsv;

    ASSERT_TRUE(nmea_parse_gsv(sentence, &gsv));

    EXPECT_EQ(gsv.sats[0].svid, 1);
    EXPECT_EQ(gsv.sats[1].svid, 2);
    EXPECT_EQ(gsv.sats[2].svid, 3);
    EXPECT_EQ(gsv.sats[3].svid, 4);
    EXPECT_EQ(gsv.sats[3].cno, 44);
}

TEST(NMEA, parse_gsv_rejects_too_many_satellite_entries)
{
    // 5 repetitions worth of fields -- exceeds the 4-satellite frame->sats capacity.
    char sentence[] =
        "$GNGSV,1,1,20,"
        "01,10,010,11,02,20,020,22,03,30,030,33,04,40,040,44,05,50,050,55,1*49\r\n";
    nmea_sentence_gsv_t gsv;

    EXPECT_FALSE(nmea_parse_gsv(sentence, &gsv));
}

// --- nmea_scan 'd'/'t' field-count boundary tests ---

TEST(NMEA, parse_rmc_zeroes_date_when_field_length_is_not_exactly_six)
{
    // Date field ("230394") shortened to 5 characters.
    char sentence[] = "$GPRMC,123519.00,A,4807.038,N,01131.000,E,022.4,084.4,23039,003.1,W,A,A*00\r\n";
    nmea_sentence_rmc_t rmc;

    ASSERT_TRUE(nmea_parse_rmc(sentence, &rmc));

    EXPECT_EQ(rmc.date.day, 0);
    EXPECT_EQ(rmc.date.month, 0);
    EXPECT_EQ(rmc.date.year, 0);
}

TEST(NMEA, parse_rmc_zeroes_time_when_field_length_is_not_exactly_nine)
{
    // Time field ("123519.00" is 9 chars) shortened to 7 characters.
    char sentence[] = "$GPRMC,1235190,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,A,A*00\r\n";
    nmea_sentence_rmc_t rmc;

    ASSERT_TRUE(nmea_parse_rmc(sentence, &rmc));

    EXPECT_EQ(rmc.time.hour, 0);
    EXPECT_EQ(rmc.time.minute, 0);
    EXPECT_EQ(rmc.time.second, 0);
    EXPECT_EQ(rmc.time.hundredth, 0);
}

TEST(NMEA, parse_gga_zeroes_time_when_field_length_is_ten)
{
    // Time field lengthened to 10 characters (one more than the required 9).
    char sentence[] = "$GPGGA,1235190.00,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*00\r\n";
    nmea_sentence_gga_t gga;

    ASSERT_TRUE(nmea_parse_gga(sentence, &gga));

    EXPECT_EQ(gga.time.hour, 0);
    EXPECT_EQ(gga.time.minute, 0);
    EXPECT_EQ(gga.time.second, 0);
    EXPECT_EQ(gga.time.hundredth, 0);
}