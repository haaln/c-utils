#ifndef CONFIG_H
#define CONFIG_H

#ifdef CXX_CLASS_IMPL
class ConfigManager
{
  public:
    void load_config();
    void configure(...); // something here
    void reload();
    void save(...); // const char* ?
    const char *get_filepath();
    const char *get_filename();

    // define else in cpp
    ConfigManager *get()
    {
        static ConfigManager s_Instance;
        return &s_Instance;
    }

    static ConfigManager s_Instance;

  private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(ConfigManager const &) = delete;
    ConfigManager &operator=(ConfigManager const &) = delete;
};
#else

#endif
#endif // !CONFIG_H
