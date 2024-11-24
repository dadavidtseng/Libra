//-----------------------------------------------------------------------------------------------
// App.hpp
//

//-----------------------------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------------------------
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
	void RequestQuit();
	void DeleteAndCreateNewGame();

	bool  m_isQuitting         = false;
	float m_timeLastFrameStart = 0.f;
};
