//----------------------------------------------------------------------------------------------------
// App.hpp
//----------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Core/EventSystem.hpp"

//----------------------------------------------------------------------------------------------------
class App
{
public:
    App();
    ~App();
    void Startup();
    void Shutdown();
    void RunFrame();

    void RunMainLoop();

private:
    void BeginFrame() const;
    void Update(float deltaSeconds);
    void Render() const;
    void EndFrame() const;

    void UpdateFromController();
    void UpdateFromKeyBoard();

    void DeleteAndCreateNewGame();
    void LoadGameConfig(char const* gameConfigXmlFilePath);


    float m_timeLastFrameStart = 0.f;
};

static bool OnWindowClose(EventArgs& arg);
static void RequestQuit();
static bool m_isQuitting;
