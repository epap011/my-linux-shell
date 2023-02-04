-Supported:
    1. Pollapla Redirections p.x ls -al > file1.txt >> file2.txt
    2. Pollapla Pipes  p.x ls -al | sort | head -2
    3. Kena anamesa se entoles metaxi ; p.x ls -al   ; pwd ; whoami;  exit

-Not Supported:
    1. Sindiasmos metaxi redirection kai pipe
    2. ta symvola |,<,>,>> prepei na exoun perikliontai apo keno xaraktira p.x ls -al>file1.txt den douleuei

-Weird Cases:
    1. mkdir "directory A" -> den tha ftiaxei ena directory me onoma directory A alla 2 directories
    2. grep  "root" -> den tha psaxei gia root alla gia to "root"
    3. Me liga logia den ginontai hangle arguments me " "..
    
H entolis echo "csd 4340" > file.txt tha grapsei csd 4340 (diladi to sosto) giati einai i moni entoli sthn
opoia exw prosthesei support gia " ".Me ligi doulitsa parapanw tha mporousa na to genikefsw gia kathe entoli typou
mkdir, rm, grep..

-Length Restrictions:
    1. The maximum number of commands seperated by ';' are 128
    2. The maximum number of arguments of each command are 64
    3. The maximum length of a line is 2048 bytes
    
-(Short Story)
1) To proramma arxika pairnei to input tou xristi
2) To input auto temaxizete se enan pinaka apo stringsopou kathe grammi antiprosopeuei
   ena apo ta isos polla commands pou edwse o xristis.Ston pinaka auton exoun apalifei ta peritta kena
3) Gia kathe grammi tou pinaka autou tis fasis 2 stelnete mia grammi(ena command) se ena function
4) To function auto an antilifthei oti prokeitai gia redirection tote petaei to mpalaki sto execution_redirection()
   An prokeitai gia pipe tote stelnei olokliri tin entoli sto execution_pipe() kai analmvanei auti
   An prokeitai gia apli entoli tote analamvanei h execution_simple()
   An prokite gia entoli built-in (cd, exit, help) tote ektelitai sigkekrimeni sinartisi gia to kathe ena apo auta.
   Etsi ki alliws den tha mporouse to child process na kanei exec to cd giati meta to parent process tha itan ekei pou itan    prin.Toidio kai me to exit.
