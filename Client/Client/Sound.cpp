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

FMOD::Channel* SoundManager::PlaySoundWithName(const char* name, float fVolume, int nLoopCount)
{
    FMOD::Channel* channel = nullptr;
    auto it = m_pSounds.find(name);
    if (it != m_pSounds.end())
    {
        m_pSystem->playSound(m_pSounds[name], nullptr, false, &channel);
        if (channel)
        {
            if (nLoopCount != 0)
            {
                channel->setLoopCount(nLoopCount);  // 반복 횟수 설정
            }
            channel->setVolume(fVolume);  // 볼륨 설정
            m_vpChannels[name].push_back(channel);  // 채널 추가
        }
    }
    return channel;
}

//void SoundManager::PauseSound(const char* name)
//{
//    auto it = m_vpChannels.find(name);
//    if (it != m_vpChannels.end())
//    {
//        for (auto& channel : it->second)
//        {
//            if (channel)
//            {
//                channel->setPaused(true);
//            }
//        }
//    }
//}
//
//void SoundManager::ResumeSound(const char* name)
//{
//    auto it = m_vpChannels.find(name);
//    if (it != m_vpChannels.end())
//    {
//        for (auto& channel : it->second)
//        {
//            if (channel)
//            {
//                channel->setPaused(false);
//            }
//        }
//    }
//}

void SoundManager::StopSound(const char* name)
{
    auto it = m_vpChannels.find(name);
    if (it != m_vpChannels.end())
    {
        for (auto& channel : it->second)
        {
            if (channel)
            {
                channel->stop();
            }
        }
        it->second.clear();  // 채널 목록 초기화
    }
}

void SoundManager::SetVolume(const char* name, float fVolume)
{
    auto it = m_vpChannels.find(name);
    if (it != m_vpChannels.end())
    {
        for (auto& channel : it->second)
        {
            if (channel)
            {
                channel->setVolume(fVolume);  // 음량 설정
            }
        }
    }
}

void SoundManager::SetVolume(FMOD::Channel* pChannel, float fVolume)
{
    if (pChannel)
    {
        pChannel->setVolume(fVolume);
    }
}

void SoundManager::UpdateSystem()
{
    if (m_pSystem)
    {
        m_pSystem->update();
    }

    // 채널 정리
    for (auto& pair : m_vpChannels)
    {
        auto& channels = pair.second;
        channels.erase(std::remove_if(channels.begin(), channels.end(), [](FMOD::Channel* channel){
            bool isPlaying = false;
            if (channel && channel->isPlaying(&isPlaying) == FMOD_OK && !isPlaying)
            {
                return true; // 채널 삭제
            }
            return false;
            }), channels.end());
    }
}

bool SoundManager::IsPlaying(const char* name)
{
    bool bPlaying = false;
    auto it = m_vpChannels.find(name);
    if (it != m_vpChannels.end())
    {
        for (auto& channel : it->second)
        {
            if (channel)
            {
                bool isPlaying = false;
                channel->isPlaying(&isPlaying);
                if (isPlaying)
                {
                    bPlaying = true;
                    break;
                }
            }
        }
    }
    return bPlaying;
}