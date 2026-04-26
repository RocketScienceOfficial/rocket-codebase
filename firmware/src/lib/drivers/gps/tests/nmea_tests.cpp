#include <gtest/gtest.h>
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