

fips_begin_app(kb-key  windowed)
    fips_files_ex(. 
                    *.cpp *.hpp
                    *.h
                    NO_RECURSE
                )
            #    fips_deps()
fips_end_app()

