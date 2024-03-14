add_rules("mode.debug", "mode.release")
set_languages("c++23")

target("devkit")
    set_kind("binary")
    add_includedirs("vendor/gsl/include")
    add_includedirs("/data/vendor/msquic/include")
    add_includedirs("/data/vendor/fmt/include")
    add_includedirs("/data/vendor/boost/include")
    add_linkdirs("/data/vendor/boost/lib") -- static
    add_linkdirs("/data/vendor/msquic/lib")
    add_linkdirs("/data/vendor/fmt/lib64") -- static
    add_rpathdirs("/data/vendor/msquic/lib") -- dynamic
    add_links("msquic", "fmt", "boost_program_options", "boost_log", "boost_thread", "pthread")
    add_files("src/**.cpp")
