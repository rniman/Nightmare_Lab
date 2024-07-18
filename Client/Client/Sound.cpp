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
    LoadSound(sound::LOBBY_SCENE, "Sound/resource/singing.wav", true);
    LoadSound(sound::MAIN_SCENE, "Sound/resource/wave.mp3", true);
    
    LoadSound(sound::GAME_WIN, "Sound/resource/town.mid", true);
    LoadSound(sound::GAME_OVER, "Sound/resource/onestop.mid", true);

    LoadSound(sound::WALK_ZOMBIE, "Sound/resource/Ding.wav");
    LoadSound(sound::ATTACK_ZOMBIE, "Sound/resource/chord.wav");
    
    LoadSound(sound::WALK_BLUESUIT, "Sound/resource/Ding.wav");
    LoadSound(sound::RUN_BLUESUIT, "Sound/resource/Ding.wav");
    LoadSound(sound::DEAD_BLUESUIT, "Sound/resource/singing.wav");
    
    LoadSound(sound::GET_ITEM_BLUESUIT, "Sound/resource/singing.wav");
    LoadSound(sound::USE_TP_BLUESUIT, "Sound//resource/singing.wav");
    LoadSound(sound::ACTIVE_MINE, "Sound/resource/Ding.wav");
    
    LoadSound(sound::USE_DRAWER, "Sound/resource/chord.wav");
    LoadSound(sound::USE_DOOR, "Sound/resource/chord.wav");
   // LoadSound(sound::USE_EXIT_DOOR, "Sound/Ding.wav");

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