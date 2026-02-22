
local PROJECT_NAME = "Core"

-- premake5.lua
workspace (PROJECT_NAME)
   configurations { "Debug", "Release" }

project (PROJECT_NAME)
   kind "StaticLib"
   language "C"
   cdialect "C99"
   targetdir "bin/%{cfg.buildcfg}"

   buildoptions { "-Wall", "-Wextra", "-Werror", "-Wpedantic" }
   links { "pthread", "curl" }

   includedirs { "." }

   files { "**.h", "**.c" }
   removefiles { "tests/**" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

newaction {
    trigger     = "build",
    description = "Build and run the project on Ubuntu",
    execute = function ()
        os.execute("premake5 gmake")
        os.execute("make")
    end
}

newaction {
    trigger     = "clean",
    description = "Clean the build folders/files on Ubuntu",
    execute = function ()
        os.execute("rm -r bin")
        os.execute("rm -r obj")
        os.execute("rm " .. PROJECT_NAME .. ".make")
        os.execute("rm Makefile")
    end
}


--[[

local function listFilesUnix(path)
    -- Ensure path ends with a slash for concatenation
    if path:sub(-1) ~= "/" then
        path = path .. "/"
    end

    local handle = io.popen('ls -A "' .. path .. '"')
    if not handle then
        error("Failed to open directory: " .. path)
    end

    local tbl = {}

    for file in handle:lines() do
        -- Skip hidden files (files starting with '.')
        if file:sub(1, 1) == "." then 
            goto continue 
        end

        local fullPath = path .. file

        -- Check if it is a directory using shell command
        -- os.execute returns true (exit code 0) if successful
        if os.execute('test -d "' .. fullPath .. '"') then
            -- It is a directory, so recurse
            local subFiles = listFilesUnix(fullPath)
            for _, subFile in ipairs(subFiles) do
                table.insert(tbl, subFile)
            end
        else
            -- It is a file, add to table
            table.insert(tbl, fullPath)
        end

        ::continue::
    end
    handle:close()

    return tbl
end


local function startswith(str, prefix)

    return str:sub(1, #prefix) == prefix
end

local function endswith(str, prefix)
    return str:sub(-#prefix) == prefix
end

newaction {
    trigger     = "test",
    description = "Test the core library",
    execute = function ()
        local files = listFilesUnix(".")

        local compile_files = ""

        local real_index = 0
        for _,v in pairs(files) do

            if not startswith(v, "./tests/test_") and not endswith(v, ".h") then
                compile_files = compile_files .. v .. " "
                goto continue
            end

            if endswith(v, ".h") then
                goto continue
            end

            --"-Wall", "-Wextra", "-Werror", "-Wpedantic"
            --links { "pthread", "curl" }

            real_index = real_index + 1
            os.execute("gcc -Wall -Wextra -Werror -Wpedantic -lpthread -lcurl " .. v .. " " .. compile_files .. " -o test_" .. real_index)

            --os.execute("gcc -Wall -Wextra -Werror -Wpedantic -lcurl -lpthread ")

            ::continue::
        end

        --print(compile_files)

    end
}

--]]
