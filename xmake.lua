set_xmakever("2.5.1")

set_languages("cxx20")
set_arch("x64")



add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("debug") then
    set_optimize("none")
elseif is_mode("releasedbg") then
    set_optimize("fastest")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
end

add_cxflags("/bigobj", "/MP")
add_defines("UNICODE")

target("ImmersiveRoleplayFramework")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX", "WINVER=0x0601")
    set_kind("shared")
    set_filename("ImmersiveRoleplayFramework.asi")
    add_files("./**.cpp")
    add_includedirs("./")
    add_syslinks("User32", "Version")
	on_package(function(target)
		
	
	end)