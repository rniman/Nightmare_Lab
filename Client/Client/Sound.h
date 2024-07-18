#pragma once
#include <fmod.hpp>

namespace sound
{
    constexpr char LOBBY_SCENE[]{ "LobbyScene" };
    constexpr char MAIN_SCENE[]{ "MainScene" };
    constexpr char GAME_WIN[]{ "GameWin" };
    constexpr char GAME_OVER[]{ "GameOver" };

    constexpr char WALK_ZOMBIE[]{ "WalkZombie" };
    constexpr char ATTACK_ZOMBIE[]{ "AttackZombie" };

    constexpr char WALK_BLUESUIT[]{ "WalkBlueSuit" };
    constexpr char RUN_BLUESUIT[]{ "RunBlueSuit" };
    constexpr char DEAD_BLUESUIT[]{ "DeadBlueSuit" };

    constexpr char GET_ITEM_BLUESUIT[]{ "GetItemBlueSuit" };
    constexpr char USE_TP_BLUESUIT[]{ "UseTpBlueSuit" };
    constexpr char ACTIVE_MINE[]{ "UseMineBlueSuit" };
    
    constexpr char USE_DRAWER[]{ "UseDrawer" };
    constexpr char USE_DOOR[]{ "UseDoor" };
    constexpr char USE_EXIT_DOOR[]{ "UseExitDoor" };
}

// 문자열 키값 비교를 위함
struct cmp_str 
{
    bool operator()(const char* a, const char* b) const 
    {
        return std::strcmp(a, b) < 0;
    }
};

class SoundManager
{
public:
    static SoundManager& GetInstance();

    bool Initialize();
    void Shutdown();

    bool LoadSound(const char* name, const char* filename, bool loop = false);
    void PlaySoundWithName(const char* name, int loopCount = 0);
    void PauseSound(const char* name);
    void ResumeSound(const char* name);
    void StopSound(const char* name);
    void SetVolume(const char* name, float volume); 

    void UpdateSystem();

    bool IsPlaying(const char* name);;

private:
    SoundManager();
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    FMOD::System* m_pSystem;
    std::map<const char*, FMOD::Sound*, cmp_str> m_pSounds;
    std::map<const char*, FMOD::Channel*, cmp_str> m_pChannels;

    char m_pErrBuffer[256];
};
