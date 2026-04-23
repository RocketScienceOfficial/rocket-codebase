extern void core_main(void);

extern "C"
{
    void app_main(void)
    {
        core_main();
    }
}