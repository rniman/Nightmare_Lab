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

    constexpr char WALK01_BLUESUIT[]{ "Walk01BlueSuit" };
    constexpr char WALK02_BLUESUIT[]{ "Walk02BlueSuit" };

    constexpr char RUN01_BLUESUIT[]{ "Run01BlueSuit" };
    constexpr char RUN02_BLUESUIT[]{ "Run02BlueSuit" };

    constexpr char DEAD_BLUESUIT[]{ "DeadBlueSuit" };

    constexpr char GET_ITEM_BLUESUIT[]{ "GetItemBlueSuit" };
    constexpr char USE_TP_BLUESUIT[]{ "UseTpBlueSuit" };
    constexpr char ACTIVE_MINE[]{ "UseMineBlueSuit" };
    
    constexpr char OPEN_DRAWER[]{ "OpenDrawer" };
    constexpr char CLOSE_DRAWER[]{ "CloseDrawer" };
    constexpr char OPEN_DOOR[]{ "OpenDoor" };
    constexpr char CLOSE_DOOR[]{ "CloseDoor" };
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
    FMOD::Channel* PlaySoundWithName(const char* name, float fVolume = 1.0f, int nLoopCount = 0);
    //void PauseSound(const char* name);
    //void ResumeSound(const char* name);
    void StopSound(const char* name);
    void SetVolume(const char* name, float fVolume);
    void SetVolume(FMOD::Channel* pChannel, float fVolume);

    void UpdateSystem();

    bool IsPlaying(const char* name);;

private:
    SoundManager();
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    FMOD::System* m_pSystem;
    std::map<const char*, FMOD::Sound*, cmp_str> m_pSounds;
    std::map<const char*, std::vector<FMOD::Channel*>, cmp_str> m_vpChannels;

    char m_pErrBuffer[256];
};
