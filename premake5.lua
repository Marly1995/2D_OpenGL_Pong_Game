
-- A solution contains projects, and defines the available configurations
solution "graphicsByExample"
   configurations { "Debug", "Release"}

   flags { "Unicode" , "NoPCH"}

   srcDirs = os.matchdirs("src/*")

   for i, projectName in ipairs(srcDirs) do

       -- A project defines one build target
       project (path.getname(projectName))
          kind "ConsoleApp"
          location (projectName)
          language "C++"
          targetdir ( projectName )

          configuration { "windows" }
             buildoptions ""
             linkoptions { "/NODEFAULTLIB:msvcrt" } -- https://github.com/yuriks/robotic/blob/master/premake5.lua
          configuration { "linux" }
             buildoptions "-std=c++11" --http://industriousone.com/topic/xcode4-c11-build-option
             toolset "gcc"
          configuration {}

          files { path.join(projectName, "**.h"), path.join(projectName, "**.cpp") } -- build all .h and .cpp files recursively
          excludes { "./graphics_dependencies/**" }  -- don't build files in graphics_dependencies/


          -- where are header files?
          configuration "windows"
          includedirs {
                        "./graphics_dependencies/SDL2/include",
						"./graphics_dependencies/SDL2_image/include",
                        "./graphics_dependencies/glew/include"
                      }
          configuration { "linux" }
          includedirs {
                        "/usr/include/SDL2",
                      }
          configuration {}


          -- what libraries need linking to
          configuration "windows"
             links { "SDL2", "SDL2main","SDL2_image", "opengl32", "glew32" }
          configuration "linux"
             links { "SDL2", "SDL2main", "GL", "GLEW" }
          configuration {}


          -- where are libraries?
          configuration "windows"
          libdirs {
                    "./graphics_dependencies/glew/lib/Release/Win32",
                    "./graphics_dependencies/SDL2/lib/win32",
					"./graphics_dependencies/SDL2_image/lib/x86"
                  }
          configuration "linux"
                   -- should be installed as in ./graphics_dependencies/README.asciidoc
          configuration {}


          configuration "*Debug"
             defines { "DEBUG" }
             flags { "Symbols" }
             optimize "Off"
             targetsuffix "-debug"


          configuration "*Release"
             defines { "NDEBUG" }
             optimize "On"
             targetsuffix "-release"


          -- copy dlls on windows
          if os.get() == "windows" then
             os.copyfile("./graphics_dependencies/glew/bin/Release/Win32/glew32.dll", path.join(projectName, "glew32.dll"))
             os.copyfile("./graphics_dependencies/SDL2/lib/win32/SDL2.dll", path.join(projectName, "SDL2.dll"))
          end
   end
