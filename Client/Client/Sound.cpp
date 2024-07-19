#include "stdafx.h"
#include "Sound.h"
#include <cstring> // strcmp 사용을 위해 추가


SoundManager::SoundManager() 
    : m_pSystem(nullptr)
{
}

SoundManager::~SoundManager()
{
    Shutdown();
}

SoundManager& SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}

bool SoundManager::Initialize() 
{
    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    if (result != FMOD_OK)
    {
        sprintf(m_pErrBuffer, "FMOD 시스템 생성 실패!\n");
        OutputDebugStringA(m_pErrBuffer);
        return false;
    }

    result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) 
    {
        sprintf(m_pErrBuffer, "FMOD 시스템 초기화 실패!\n");
        OutputDebugStringA(m_pErrBuffer);
        return false;
    }

    // 모든 사운드 로드
    LoadSound(sound::LOBBY_SCENE, "Sound/resource/lobby_bgm.wav", true);
    LoadSound(sound::MAIN_SCENE, "Sound/resource/main_bgm.wav", true);
    
    LoadSound(sound::GAME_WIN, "Sound/resource/game_win.wav", false);
    LoadSound(sound::GAME_OVER, "Sound/resource/game_fail.wav", false);

    LoadSound(sound::WALK_ZOMBIE, "Sound/resource/Zombie_Walk.wav");
    LoadSound(sound::ATTACK_ZOMBIE, "Sound/resource/Zombie_Attack.wav");
    
    LoadSound(sound::WALK01_BLUESUIT, "Sound/resource/Walk01_BlueSuit.wav");
    LoadSound(sound::WALK02_BLUESUIT, "Sound/resource/Walk02_BlueSuit.wav");
    LoadSound(sound::RUN01_BLUESUIT, "Sound/resource/Walk01_BlueSuit.wav");
    LoadSound(sound::RUN02_BLUESUIT, "Sound/resource/Walk02_BlueSuit.wav");

    LoadSound(sound::DEAD_BLUESUIT, "Sound/resource/BlueSuit_Dead.wav");
    
    LoadSound(sound::GET_ITEM_BLUESUIT, "Sound/resource/Get_Item.wav");
    LoadSound(sound::USE_TP_BLUESUIT, "Sound//resource/Teleport.wav");
    LoadSound(sound::ACTIVE_MINE, "Sound/resource/Mine.wav");
    
    LoadSound(sound::OPEN_DRAWER, "Sound/resource/Open_Drawer.wav");
    LoadSound(sound::CLOSE_DRAWER, "Sound/resource/Close_Drawer (mp3cut.net).wav");
    LoadSound(sound::OPEN_DOOR, "Sound/resource/Open_Door.wav");
    LoadSound(sound::CLOSE_DOOR, "Sound/resource/Close_Door.wav");

    return true;
}

void SoundManager::Shutdown()
{
    for (auto& sound : m_pSounds) 
    {
        sound.second->release();
    }
    m_pSounds.clear();

    if (m_pSystem) 
    {
        m_pSystem->close();
        m_pSystem->release();
        m_pSystem = nullptr;
    }
}

bool SoundManager::LoadSound(const char* name, const char* filename, bool loop) 
{
    FMOD::Sound* sound = nullptr;
    FMOD_RESULT result = m_pSystem->createSound(filename, loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, nullptr, &sound);
    if (result != FMOD_OK)
    {
        sprintf(m_pErrBuffer, "사운드 로드 실패 : %s \n", filename);
        OutputDebugStringA(m_pErrBuffer);
        return false;
    }

    m_pSounds[name] = sound;
    return true;
}

void SoundManager::PlaySoundWithName(const char* name, int loopCount)
{
    auto it = m_pSounds.find(name);
    if (it != m_pSounds.end())
    {
        FMOD::Channel* channel = nullptr;
        m_pSystem->playSound(m_pSounds[name], nullptr, false, &channel);
        if (channel && loopCount != 0) 
        {
            channel->setLoopCount(loopCount);  // 반복 횟수 설정
        }
        m_pChannels[name] = channel;
    }
}

void SoundManager::PauseSound(const char* name)
{
    auto it = m_pChannels.find(name);
    if (it != m_pChannels.end() && it->second) 
    {
        it->second->setPaused(true);
    }
}

void SoundManager::ResumeSound(const char* name)
{
    auto it = m_pChannels.find(name);
    if (it != m_pChannels.end() && it->second)
    {
        it->second->setPaused(false);
    }
}

void SoundManager::StopSound(const char* name)
{
    auto it = m_pChannels.find(name);
    if (it != m_pChannels.end() && it->second)
    {
        it->second->stop();
    }
}

void SoundManager::SetVolume(const char* name, float volume)
{
    auto it = m_pChannels.find(name);
    if (it != m_pChannels.end() && it->second)
    {
        it->second->setVolume(volume);  // 음량 설정
    }
}

void SoundManager::UpdateSystem() 
{
    if (m_pSystem)
    {
        m_pSystem->update();
    }
}

bool SoundManager::IsPlaying(const char* name)
{ 
    bool bPlaying = false;
    m_pChannels[name]->isPlaying(&bPlaying);
    return bPlaying;
}