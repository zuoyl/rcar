#!/bin/bash

case $1 in
    start)
        echo "start... nginx"
        /usr/local/nginx/sbin/nginx -c /usr/local/nginx/conf/nginx.conf

        echo "start... mongodb"
        /usr/local/mongo/bin/mongod &
        
        ;;
    stop)
        echo "stop... nginx"
        killall nginx

        echo "stop... mongodb"
        killall mongod

        ;;
     *)
        echo "invalid command line syntax"
esac
exit 0
