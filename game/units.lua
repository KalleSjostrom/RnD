Program {
   Name = "Game",
   Sources = { "source/game.cpp" },
   ReplaceEnv = {
      LD = { "$(CXX)" ; Config = { "*-win32-msvc-*" } },
   },
}

Default "Game"
