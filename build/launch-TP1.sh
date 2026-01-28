#!/bin/sh
bindir=$(pwd)
cd /home/jerem/Documents/M1_IMAGINE_2025-2026/Semestre8/Moteur_De_Jeux/TP1/HAI819I_TP1/TP1/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		GDB_COMMAND-NOTFOUND -batch -command=$bindir/gdbscript  USERFILE_COMMAND-NOTFOUND 
	else
		"USERFILE_COMMAND-NOTFOUND"  
	fi
else
	"USERFILE_COMMAND-NOTFOUND"  
fi
