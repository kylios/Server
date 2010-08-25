#! /usr/bin/perl

# Main perl launcher script for the server.  This script sets everything
# up, then gives control to the user command script.  
#
# At this point, we should be in a child process with connections to
# the client and read/write connections to the parent process.

open LOGFILE, ">>", "/home/kyle/Projects/server/test/logfile" or die $!;
open ERRFILE, ">>", "/home/kyle/Projects/server/test/errfile" or die $!;

print LOGFILE "Argv:\n" .
        $ARGV[0] . "\n" .
        $ARGV[1] . "\n" .
        $ARGV[2] . "\n" .
        $ARGV[3] . "\n" .
        $ARGV[4] . "\n";

