SUB SUBR X,Y,Z	
	LOCAL	N
	N=	10
	FORMAT	"SUBR: %,%,%", X,Y,Z
	FORMAT	"SUBR: OK!"
	RETURN
	FORMAT	"S: RETURN DID NOT RETURN!"
END SUB

SUB F	X,Y
	RETURN	X+Y
	FORMAT	"F: RETURN DID NOT RETURN!"
END SUB

N=	400
X=	100
Y=	200
Z=	300
SUBR	1,2,3
FORMAT	"X,Y,Z,N: %,%,%,%", X,Y,Z,N
FORMAT	"F(100,200): %", F(100,F(120,80))

I=	0
WHILE	I<5
	FORMAT	"WHILE I:%",I
	I=I+1
END WHILE

SUB TESTLOOP
	LOCAL I
	FOR I=0 TO 3
	END FOR
END SUB
FOR	I=0 TO 5
	TESTLOOP
END FOR

FOR	I=0 TO 3
FOR	J=10 TO 12
	FORMAT	"% %", I,J
END FOR
END FOR

SUB	TESTESCAPE
	FOR	I=0 TO 10
		RETURN
	END FOR
END SUB
TESTESCAPE

IF	0
	FORMAT	"NO"
ELSE IF 1<2
	FORMAT	"YES!"
ELSE
	FORMAT "WTF"
END IF

IF	1 THEN FORMAT "IN-LINE OK"
IF	0 THEN FORMAT "IN-LINE BAD"

DIM	A(10)
FOR	I=1 TO UBOUND(A)+1
	A(I)=	I*10
END FOR
FORMAT	"% % %",A(1),A(10),UBOUND(A)

#FORMAT	"RESUMED BOUND: %", A(0)
#FORMAT "RESUMED DIV: %", 1/0
#BREAK
FORMAT	"DONE."