#!/bin/bash
function -r {
    premake5 gmake;
    make config=release all;
}

function -d {
    premake5 gmake;
    make config=debug all;
}

function -a {
    bash .build_assets.sh;
}

function -c {
    make clean;
    echo -e "=====Cleaning done=====";
}


function -h {
    echo -e "
    ==========================================================================
    run ./build.sh -r to compile for release
    run ./build.sh -d to compile for debug
    run ./build.sh -c to run make clean (start fresh)
    run ./build.sh -h to read this msg :3
    ==========================================================================
    ";
}


### command list ###
case $1 in
    -r)
    -r
    ;;

    -d)
    -d
    ;;

    -a)
    -a
    ;;

    -c)
    -c
    ;;

    -h)
    -h
    ;;

    *)
    echo -e "
    ==========================================================================
    run ./build.sh -r to compile for release
    run ./build.sh -d to compile for debug
    run ./build.sh -c to run make clean (start fresh)
    run ./build.sh -h to read this msg :3
    ==========================================================================
    "

esac
