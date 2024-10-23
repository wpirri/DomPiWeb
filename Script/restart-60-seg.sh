#!/bin/sh

go_restart()
{
        sleep 60
        init 6
}

go_restart&

