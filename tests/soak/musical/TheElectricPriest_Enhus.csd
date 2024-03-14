<CsoundSynthesizer>
<CsOptions>
; Select audio/midi flags here according to platform
-odac   ;;;realtime audio out
;-iadc    ;;;uncomment -iadc if realtime audio input is needed too
; For Non-realtime ouput leave only the line below:
; -o TheElectricPriest_Enhus.wav -W ;;; for file output any platform
</CsOptions>
<CsInstruments>

;excerpt from 'The Electric Priest' by Tobias Enhus

sr=44100
ksmps=1
nchnls=2

 garvbsig init 0
;-------------------------------------------------
; ********** The Electric Priest ***" Talking "***
;-------------------------------------------------
;------------------------------------------
; ********** The Electric Priest ***Aaah**
;------------------------------------------
;-------------------------------------------------
; ********** The Electric Priest ***" gliss "***
;-------------------------------------------------
;*******************************************************************************************
;|||||||||||||||||
;||-Sonar Noise-||
;|||||||||||||||||
	
; Soft OSCIL/fof instrument, p3:dur p4:amp p5:pitch p6:reverb p7:att p8:rel p9:bal
; Bass OSCIL/FM instrument, p3:dur p4:amp p5:pitch p6:reverb p7:att p8:rel p9:bal
;'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
; Pan Noise
;'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
;'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
; Hi-Hat Noise 
;'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
;'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
;______
;METAL
;------
; Kick Drum 
; Global Reverb


	instr 1	;
inote=cpspch(p5)
irvbgain = p8
k2 	linseg  	0, p3*.9, 0, p3*.1, 1  			; octaviation coefficient
a1	oscil 	7, .15,1					;Rubato for vibrato
a3	linen		a1, (p3-p3*.05), p3, .2			;delay for vibrato
a2	oscil 	a3, 5,1					;vibrato
a5	linen		1250, p7, p3, (p3*.1)			;amp envelope
a21	line	456, p6, 1030					; p6: morph-time,0=instant aah					
a5	linen		10000, p7, p3, (p3*.1)			;amp envelope
a11 fof	a5,inote+a2, a21*(p4/100), k2, 200, .003, .017, .005, 10, 1,2, inote, 0, 1
a31	line	4000, p6, 6845
a32	line	2471, p6, 1370										
a6	linen		a31, p7, p3, (p3*.1)			;amp envelope
a12 fof      a6,inote+a2, a32*(p4/100), k2, 200, .003, .017, .005, 20, 1,2, inote, 0, 1
a41	line	2813, p6, 3170
a42	line	1650, p6, 1845										
a7	linen		a42, p7, p3, (p3*.1)			;amp envelope
a13 fof      a7,inote+a2, a41*(p4/100), k2, 200, .003, .017, .005, 20, 1,2, inote, 0, 1
a71	line	1347, p6, 1726 	;amp line
a72	line	3839, p6, 3797	; form line										
a8	linen		a71, p7, p3, (p3*.1)			;amp envelope
a14 fof      a8,inote+a2, a72*(p4/100), k2, 200, .003, .017, .005, 30, 1,2, inote, 0, 1
a51	line	1, p6, 1250										
a9	linen		a51, p7, p3, (p3*.1)			;amp envelope
a15 fof      a5,inote+a2, 4177*(p4/100), k2, 200, .003, .017, .005, 30, 1,2, inote, 0, 1
a61	line	1, p6, 5833
a10	linen		a51, p7, p3, (p3*.1)			;amp envelope
a16 fof      a10,inote+a2,  428*(p4/100), k2, 200, .003, .017, .005, 10, 1,2, inote, 0, 1
a7 =        (a11 + a12 + a13 + a14 + a15 + a16) * p9 / 10
outs  a7*.9,a7*.6
garvbsig = garvbsig+(a7)*irvbgain

	endin

	instr 2	;
inote=cpspch(p5)
irvbgain = p8
k2 	linseg  	0, p3*.9, 0, p3*.1, 1  			; octaviation coefficient
a1	oscil 	5, .12,1					;Rubato for vibrato
a3	linen		a1, (p3-p3*.02), p3, .2			;delay for vibrato
a2	oscil 	a3, 4,1					;vibrato
a4	linen		(p4*.4), p6, p3, (p3*.05)		;format env shape
a5	linen		1250, p7, p3, (p3*.15)			;amp envelope
a21	oscil 	2, 10,1					
a5	linen		9998+a21, p7, p3, (p3*.1)			;amp envelope
a11 fof	a5,inote+a2*.5, a4+1030*(p4/100), k2, 200, .003, .017, .005, 10, 1,2, inote, 0, 1
a22	oscil 	2, 2,1
a6	linen		6843+a22, p7, p3, (p3*.1)			;amp envelope
a12 fof      a6,inote+a2*.5, a4+1370*(p4/100), k2, 200, .003, .017, .005, 20, 1,2, inote, 0, 1
a23	oscil 	1, 12,1
a7	linen		1845, p7, p3, (p3*.1)			;amp envelope
a13 fof      a7,inote+a2*.5, a4+3170*(p4/100), k2, 200, .003, .017, .005, 20, 1,2, inote, 0, 1
a24	oscil 	2, 5,1
a8	linen		1726+a24, p7, p3, (p3*.1)			;amp envelope
a14 fof      a8,inote+a2*.5, a4+3797*(p4/100), k2, 200, .003, .017, .005, 30, 1,2, inote, 0, 1
a25	oscil 	3, 4,1
a9	linen		1250+a25, p7, p3, (p3*.1)			;amp envelope
a15 fof      a5,inote+a2*.5, a4+4177*(p4/100), k2, 200, .003, .017, .005, 30, 1,2, inote, 0, 1
a26	oscil 	3, 6,1
a10	linen		5833+a26, p7, p3, (p3*.1)			;amp envelope
a16 fof      a10,inote+a2*.5,  a4+428*(p4/100), k2, 200, .003, .017, .005, 10, 1,2, inote, 0, 1
a7 =        (a11 + a12 + a13 + a14 + a15 + a16) * p9 / 10
outs  a7*p6,a7*(1-p6)
garvbsig = garvbsig+(a7)*irvbgain

	endin

	instr 3	;
irvbgain = p6
inote=cpspch(p5)
ibalance=	p9	
k1	oscil 	300, .2,1				
k2	oscil		200, .5,1
k3	oscil		400, .05,1				; res sweep
a1	oscil		p4*0.30, inote*0.998, 4
a2	oscil		p4*0.30, inote*1.002, 4
a3	oscil		p4*0.20, inote*1.000, 5
a4   	fof  		p4*0.20+k2, inote, 600, 0, 40, .003, .02, .007, 10, 1, 2, p3
amix= a1+a2+a3+a4
a6	butterlp	amix,k1+1000,50		;input, freq, Q
a7	butterlp	a6, k1+1000,50
a8	butterhp	a7, 600	
a9	reson		a8, k3+2000, 50
a10	butterhp	a9*.2+a8*1, 150	
a11	butterlp	a10,9000
aenv	linen		a11, p7, p3, p8
outs aenv*ibalance, aenv*(1-ibalance)
garvbsig = garvbsig+(aenv)*irvbgain

	endin

	instr 10	;
irvbgain = p6
inote=cpspch(p5)
ibalance=	p9	
k1	line	 	2500, p3*.8,1500				
k3	line	 	6000, p3*.5,500				; res sweep
k2	linen		k3, .08, p3*.5, .5
a1	oscil		p4*0.40, inote*0.998-.12, 06
a2	oscil		p4*0.40, inote*1.002-.12, 03
a3	oscil		p4*0.40, inote*1.002-.12, 06
a4	oscil		p4*0.70, inote-.24, 03
amix= a1+a2+a3+a4
a6	butterlp	amix,k2,20		;input, freq, Q
a8	butterlp	(a6), k2,20
a9	butterhp	a8, 65	
a10	butterhp	a9, 65	
a11	butterlp	a10,1000
aenv	linen		a11, p7, p3, p8
outs aenv*ibalance, aenv*(1-ibalance)
garvbsig = garvbsig+(aenv)*irvbgain

	endin

	instr 21	;
irvbgain = p6
i1	=	octpch(p5)			; center pitch, to be modified
k1	randh	10,10, .5				; 10 time/sec by random displacements up to 1 octave
a1	oscil	p7, cpsoct(i1+k1), 10
a2	reson		a1,1000,50
a3	butterlp	a2*.4+a1,2500
a4	butterlp	a3,2500
a5	butterhp	a4,950
a6	butterhp	a5,950
aenv	linen		a6, p3*.2, p3, p3*2
a7	oscil	1, .4, 03
outs aenv*a7, aenv*(1-a7)
garvbsig = garvbsig+a1*irvbgain

	endin

	instr 22	;
irvbgain = p6
ibalance=	p4
k1	expon		10000, .1, 2500
a0	expon		p7+150, .1, 10
a1	rand		a0
a3	butterlp	a1,k1
a4	butterlp	a3,k1
a5	butterhp	a4,3500
a6	butterhp	a5,3500
a8	linen		(a6+a1),0,p3, .05  
outs a8*ibalance, a8*(1-ibalance)
garvbsig = garvbsig+a1*irvbgain

	endin

	instr 23	;
irvbgain = p6
ibalance=	p4
i1	=	octpch(p5)			; center pitch, to be modified
a1	fmmetal	31129.60*p7, p5, 1,   1.2, 0.2, 5.5, 1,5,5,1, 1
a8	linen		a1,.01,p3, p3*.3  
outs a8*ibalance, a8*(1-ibalance)
garvbsig = garvbsig+a1*irvbgain

	endin

	instr 99	;
irvbtime= p4
asig reverb garvbsig, irvbtime
outs asig,asig
garvbsig = 0

	endin

</CsInstruments>
<CsScore>
f1  0   4096 10  1					;fof
f2  0   1024 19  .5  .5  270  .5			;fof
f03 0   1024 10  1					;sine
f04 0   1024 7  0 124 1 900 0				;saw
f05 0   1024 7  0 124 1 900 1 1 1 1 1 1 1 1 	;Buzz
f06 0   2048 10 1 1 1 1 .7 .5 .3 .1 		;pulse
f10 0   1024   21  1					;noise


i1	0.75	3.2	70	7.02	6	3.5	.1	6
i1	3.950000000000003	0.4	70	7.02	20	.1	.07	6
i1	4.3500000000000085	0.4	70	7.02	.3	.1	.07	6
i1	4.750000000000014	0.4	70	7.04	20	.1	.07	6
i1	5.15000000000002	0.4	70	7.05	.6	.1	.07	6
i1	5.550000000000026	0.4	70	7.05	20	.1	.07	6
i1	5.950000000000031	1.6	70	7.07	2	.8	.07	6
i1	7.550000000000026	3.2	70	7.05	200	.4	.07	6
i1	10.750000000000028	0.8	70	7.10	11	.2	.0	0
i1	11.550000000000026	4.4	70	7.10	11	1.8	.2	3
i1	15.950000000000031	0.8	70	7.12	.4	.05	.2	3
i1	16.75000000000003	6.4	70	7.09	13	.1	.2	3
i2	0.75	22.4	70	6.02	.5	11	.13	1.3
i2	0.75	1.6	70	8.02	.3	.8	.1	2
i2	2.3500000000000014	1.2	70	8.00	.5	.2	.1	2
i2	3.5500000000000043	0.4	70	7.10	.5	.1	.1	2
i2	3.950000000000003	2	70	7.09	.7	.3	.1	2
i2	6.75	0.8	70	8.02	.7	.3	.1	2
i2	7.549999999999997	0.4	70	8.04	.7	.1	.1	2
i2	7.950000000000003	0.4	70	8.02	.7	.1	.1	2
i2	8.350000000000009	3.2	70	8.01	.7	.4	.1	1.5
i2	11.550000000000011	3.2	70	8.00	.7	.6	.1	1.1
i2	14.750000000000014	3.2	70	8.00	.7	.9	.1	1
i3	0.050000000000174794	0.1	12000	8.03	.0	.01	.01	0.8571428571428572
i3	0.15000000000017621	0.1	12000	8.03	.0	.01	.01	0.7142857142857143
i3	0.25000000000017764	0.1	12000	8.03	.0	.01	.01	0.5714285714285714
i3	0.35000000000017906	0.1	12000	8.03	.0	.01	.01	0.4285714285714286
i3	0.4500000000001805	0.1	12000	8.03	.0	.01	.01	0.2857142857142858
i3	0.5500000000001819	0.1	12000	8.03	.0	.01	.01	0.1428571428571429
i3	0.6500000000001833	0.1	12000	8.03	.0	.01	.01	0
i3	0.75	22.4	3200	7.02	.1	.1	2	.1
i3	0.75	22.4	3200	7.05	.1	.1	2	.9
i3	0.75	22.4	3200	7.09	.1	.1	2	.5
i3	0.75	22.4	3200	7.12	.1	.1	2	.2
i3	3.950000000000003	1.6	5200	8.02	.05	.1	.5	.6
i3	5.549999999999997	1.2	5200	8.00	.05	.1	.5	.6
i3	6.75	0.4	5200	7.10	.05	.1	.5	.4
i3	7.150000000000006	2	5200	7.09	.05	.1	.5	.2
i10	0.3501499999998714	0.2	9500	6.00	.01	.02	.01	.5
i10	0.5501499999998742	0.2	9500	6.00	.01	.02	.01	.5
i10	0.75	0.4	9500	5.02	.01	.02	.01	.5
i10	1.1499999999999915	0.4	9500	5.02	.01	.02	.01	.5
i10	1.5499999999999972	0.26667	9500	5.03	.01	.02	.01	.5
i10	1.8166699999999878	0.26667	9500	5.03	.01	.02	.01	.5
i10	2.0833399999999926	0.26667	9500	5.03	.01	.02	.01	.5
i10	2.3500099999999833	0.4	9500	5.02	.01	.02	.01	.5
i10	2.750009999999989	0.4	9500	5.02	.01	.02	.01	.5
i10	3.1500099999999804	0.4	9500	5.02	.01	.02	.01	.5
i10	3.550009999999986	0.2	9500	6.02	.01	.02	.01	.5
i10	3.750009999999989	0.2	9500	6.02	.01	.02	.01	.5
i10	3.950009999999992	0.4	9500	5.02	.01	.02	.01	.5
i10	4.3500099999999975	0.4	9500	5.02	.01	.02	.01	.5
i10	4.750010000000003	0.26667	9500	5.03	.01	.02	.01	.5
i10	5.016680000000008	0.26667	9500	5.03	.01	.02	.01	.5
i10	5.283350000000013	0.26667	9500	5.03	.01	.02	.01	.5
i10	5.550020000000018	0.4	9500	5.02	.01	.02	.01	.5
i10	5.950020000000023	0.4	9500	5.02	.01	.02	.01	.5
i10	6.350020000000029	0.4	9500	5.02	.01	.02	.01	.5
i10	6.750020000000035	0.4	9500	5.12	.01	.02	.01	.5
i10	7.1500200000000405	0.4	9500	6.02	.01	.02	.01	.5
i10	7.550020000000046	0.4	9500	5.02	.01	.02	.01	.5
i10	7.950020000000052	0.26667	9500	5.03	.01	.02	.01	.5
i10	8.216690000000057	0.26667	9500	5.03	.01	.02	.01	.5
i10	8.483360000000062	0.26667	9500	5.03	.01	.02	.01	.5
i10	8.750030000000066	0.4	9500	5.02	.01	.02	.01	.5
i10	9.150030000000072	0.4	9500	5.02	.01	.02	.01	.5
i10	9.550030000000078	0.4	9500	5.02	.01	.02	.01	.5
i10	9.950030000000083	0.2	9500	6.02	.01	.02	.01	.5
i10	10.150030000000086	0.2	9500	6.02	.01	.02	.01	.5
i10	10.350030000000089	0.4	9500	5.02	.01	.02	.01	.5
i10	10.75	0.1	7000	10.02	.01	.02	.01	.8
i10	10.750030000000095	0.4	9500	5.02	.01	.02	.01	.5
i10	10.849999999999994	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	10.949999999999989	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	11.049999999999983	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	11.149999999999977	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	11.1500300000001	0.26667	9500	5.03	.01	.02	.01	.5
i10	11.34999999999998	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	11.416700000000105	0.26667	9500	5.03	.01	.02	.01	.5
i10	11.449999999999974	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	11.549999999999969	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	11.649999999999963	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	11.68337000000011	0.26667	9500	5.03	.01	.02	.01	.5
i10	11.749999999999957	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	11.849999999999952	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	11.949999999999946	0.4	7000	10.02	.01	.02	.01	.3
i10	11.950040000000115	0.4	9500	5.02	.01	.02	.01	.5
i10	12.349999999999952	0.1	7000	10.02	.01	.02	.01	.8
i10	12.35004000000012	0.4	9500	5.02	.01	.02	.01	.5
i10	12.449999999999946	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	12.54999999999994	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	12.649999999999935	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	12.749999999999929	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	12.750040000000126	0.4	9500	5.02	.01	.02	.01	.5
i10	12.949999999999932	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	13.049999999999926	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	13.14999999999992	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	13.150040000000132	0.4	9500	5.12	.01	.02	.01	.5
i10	13.249999999999915	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	13.349999999999909	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	13.449999999999903	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	13.549999999999898	0.4	7000	10.02	.01	.02	.01	.3
i10	13.550040000000138	0.4	9500	6.02	.01	.02	.01	.5
i10	13.949999999999903	0.1	7000	10.02	.01	.02	.01	.8
i10	13.950040000000143	0.4	9500	5.02	.01	.02	.01	.5
i10	14.049999999999898	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	14.149999999999892	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	14.249999999999886	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	14.34999999999988	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	14.35004000000015	0.26667	9500	5.03	.01	.02	.01	.5
i10	14.549999999999883	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	14.616710000000154	0.26667	9500	5.03	.01	.02	.01	.5
i10	14.649999999999878	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	14.749999999999872	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	14.849999999999866	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	14.883380000000159	0.26667	9500	5.03	.01	.02	.01	.5
i10	14.94999999999986	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	15.049999999999855	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	15.14999999999985	0.4	7000	10.02	.01	.02	.01	.3
i10	15.150050000000164	0.4	9500	5.02	.01	.02	.01	.5
i10	15.549999999999855	0.1	7000	10.02	.01	.02	.01	.8
i10	15.55005000000017	0.4	9500	5.02	.01	.02	.01	.5
i10	15.64999999999985	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	15.749999999999844	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	15.849999999999838	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	15.949999999999832	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	15.950050000000175	0.4	9500	5.02	.01	.02	.01	.5
i10	16.149999999999835	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	16.24999999999983	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	16.349999999999824	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	16.35005000000018	0.2	9500	6.02	.01	.02	.01	.5
i10	16.449999999999818	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	16.549999999999812	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	16.550050000000184	0.2	9500	6.02	.01	.02	.01	.5
i10	16.649999999999807	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	16.7499999999998	0.4	7000	10.02	.01	.02	.01	.3
i10	16.750050000000186	0.4	9500	5.02	.01	.02	.01	.5
i10	17.149999999999807	0.1	7000	10.02	.01	.02	.01	.8
i10	17.150050000000192	0.4	9500	5.02	.01	.02	.01	.5
i10	17.2499999999998	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	17.349999999999795	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	17.44999999999979	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	17.549999999999784	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	17.550050000000198	0.26667	9500	5.03	.01	.02	.01	.5
i10	17.749999999999787	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	17.816720000000203	0.26667	9500	5.03	.01	.02	.01	.5
i10	17.84999999999978	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	17.949999999999775	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	18.04999999999977	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	18.083390000000207	0.26667	9500	5.03	.01	.02	.01	.5
i10	18.149999999999764	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	18.24999999999976	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	18.349999999999753	0.4	7000	10.02	.01	.02	.01	.3
i10	18.350060000000212	0.4	9500	5.02	.01	.02	.01	.5
i10	18.74999999999976	0.1	7000	10.02	.01	.02	.01	.8
i10	18.750060000000218	0.4	9500	5.02	.01	.02	.01	.5
i10	18.849999999999753	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	18.949999999999747	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	19.04999999999974	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	19.149999999999736	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	19.150060000000224	0.4	9500	5.02	.01	.02	.01	.5
i10	19.34999999999974	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	19.449999999999733	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	19.549999999999727	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	19.55006000000023	0.4	9500	5.12	.01	.02	.01	.5
i10	19.64999999999972	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	19.749999999999716	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	19.84999999999971	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	19.949999999999704	0.4	7000	10.02	.01	.02	.01	.3
i10	19.950000000000003	0.1	9000	12.02	.0	.02	.01	0
i10	19.950060000000235	0.4	9500	6.02	.01	.02	.01	.5
i10	20.049999999999997	0.1	9000	12.01	0.006451612903225807	.02	.01	0.03225806451612903
i10	20.14999999999999	0.1	9000	12.00	0.012903225806451615	.02	.01	0.06451612903225806
i10	20.249999999999986	0.1	9000	11.11	0.01935483870967742	.02	.01	0.0967741935483871
i10	20.34999999999971	0.1	7000	10.02	.01	.02	.01	.8
i10	20.34999999999998	0.1	9000	11.10	0.02580645161290323	.02	.01	0.12903225806451613
i10	20.35006000000024	0.4	9500	5.02	.01	.02	.01	.5
i10	20.449999999999704	0.1	7000	10.02	.01	.02	.01	0.7583333333333339
i10	20.449999999999974	0.1	9000	11.09	0.03225806451612904	.02	.01	0.16129032258064516
i10	20.5499999999997	0.1	7000	10.02	.01	.02	.01	0.7166666666666677
i10	20.54999999999997	0.1	9000	11.08	0.03870967741935484	.02	.01	0.1935483870967742
i10	20.649999999999693	0.1	.0	10.02	.01	.02	.01	0.6750000000000015
i10	20.649999999999963	0.1	9000	11.07	0.04516129032258065	.02	.01	0.22580645161290322
i10	20.749999999999687	0.2	7000	10.02	.01	.02	.01	0.6333333333333353
i10	20.749999999999957	0.1	9000	11.06	0.05161290322580646	.02	.01	0.25806451612903225
i10	20.750060000000246	0.26667	9500	5.03	.01	.02	.01	.5
i10	20.84999999999995	0.1	9000	11.05	0.05806451612903227	.02	.01	0.29032258064516125
i10	20.94999999999969	0.1	7000	10.02	.01	.02	.01	0.5499999999999972
i10	20.949999999999946	0.1	9000	11.04	0.06451612903225808	.02	.01	0.3225806451612903
i10	21.01673000000025	0.26667	9500	5.03	.01	.02	.01	.5
i10	21.049999999999685	0.1	7000	10.02	.01	.02	.01	0.5083333333333309
i10	21.04999999999994	0.1	9000	11.03	0.07096774193548389	.02	.01	0.3548387096774193
i10	21.14999999999968	0.1	7000	10.02	.01	.02	.01	0.46666666666666473
i10	21.149999999999935	0.1	9000	11.02	0.07741935483870968	.02	.01	0.3870967741935484
i10	21.249999999999673	0.1	7000	10.02	.01	.02	.01	0.4249999999999986
i10	21.24999999999993	0.1	9000	11.01	0.08387096774193549	.02	.01	0.4193548387096774
i10	21.283400000000256	0.26667	9500	5.03	.01	.02	.01	.5
i10	21.349999999999667	0.1	7000	10.02	.01	.02	.01	0.3833333333333324
i10	21.349999999999923	0.1	9000	11.0	0.0903225806451613	.02	.01	0.45161290322580644
i10	21.449999999999662	0.1	.0	10.02	.01	.02	.01	0.34166666666666623
i10	21.449999999999918	0.1	9000	10.11	0.09677419354838711	.02	.01	0.48387096774193544
i10	21.549999999999656	0.4	7000	10.02	.01	.02	.01	.3
i10	21.549999999999912	0.1	9000	10.10	0.10322580645161292	.02	.01	0.5161290322580645
i10	21.55007000000026	0.4	9500	5.02	.01	.02	.01	.5
i10	21.649999999999906	0.1	9000	10.09	0.10967741935483873	.02	.01	0.5483870967741935
i10	21.7499999999999	0.1	9000	10.08	0.11612903225806454	.02	.01	0.5806451612903225
i10	21.849999999999895	0.1	9000	10.07	0.12258064516129034	.02	.01	0.6129032258064515
i10	21.949999999999662	0.1	7000	10.02	.01	.02	.01	0.3
i10	21.94999999999989	0.1	9000	10.06	0.12903225806451615	.02	.01	0.6451612903225806
i10	21.950070000000267	0.4	9500	5.02	.01	.02	.01	.5
i10	22.049999999999656	0.1	7000	10.02	.01	.02	.01	0.3
i10	22.049999999999883	0.1	9000	10.05	0.13548387096774195	.02	.01	0.6774193548387096
i10	22.14999999999965	0.1	7000	10.02	.01	.02	.01	0.3
i10	22.149999999999878	0.1	9000	10.04	0.14193548387096777	.02	.01	0.7096774193548386
i10	22.249999999999645	0.1	.0	10.02	.01	.02	.01	0.3
i10	22.249999999999872	0.1	9000	10.03	0.14838709677419357	.02	.01	0.7419354838709676
i10	22.34999999999964	0.4	7000	10.02	.01	.02	.01	.3
i10	22.349999999999866	0.1	9000	10.02	0.15483870967741936	.02	.01	0.7741935483870968
i10	22.350070000000272	0.4	9500	5.02	.01	.02	.01	.5
i10	22.44999999999986	0.1	9000	10.01	0.16129032258064518	.02	.01	0.8064516129032258
i10	22.549999999999855	0.1	9000	10.00	0.16774193548387098	.02	.01	0.8387096774193548
i10	22.64999999999985	0.1	9000	9.11	0.1741935483870968	.02	.01	0.8709677419354838
i10	22.749999999999844	0.1	9000	9.10	0.1806451612903226	.02	.01	0.9032258064516129
i10	22.750070000000278	0.2	9500	6.02	.01	.02	.01	.5
i10	22.849999999999838	0.1	9000	9.09	0.18709677419354842	.02	.01	0.9354838709677419
i10	22.949999999999832	0.1	9000	9.08	0.19354838709677422	.02	.01	0.9677419354838709
i10	22.95007000000028	0.2	9500	6.02	.01	.02	.01	.5
i10	23.049999999999827	0.1	9000	9.07	.2	.02	.01	1
i21	13.838888888888889	13.6111111111	.5	8.00	.01	200
i22	0.050000000000096634	0.1	0.25	6.00	.01	300
i22	0.15000000000009095	0.1	0.16666666666666663	6.00	.01	300
i22	0.2500000000000995	0.1	0.08333333333333337	6.00	.01	300
i22	0.3500000000000938	0.05	0	6.00	.01	300
i22	0.40000000000009095	0.05	0.14285714285714288	6.00	.01	300
i22	0.4500000000000881	0.05	0.28571428571428575	6.00	.01	300
i22	0.5000000000000853	0.05	0.4285714285714286	6.00	.01	300
i22	0.5500000000000824	0.05	0.5714285714285715	6.00	.01	300
i22	0.6000000000000796	0.05	0.7142857142857143	6.00	.01	300
i22	0.6500000000000767	0.05	0.8571428571428572	6.00	.01	300
i22	0.7000000000000739	0.05	1	6.00	.01	300
i22	0.75	0.1	0	6.00	.01	300
i22	0.8500000000000085	0.1	0.06666666666666667	6.00	.01	300
i22	0.9500000000000028	0.1	0.13333333333333333	6.00	.01	300
i22	1.0500000000000114	0.1	0.2	6.00	.01	300
i22	1.1500000000000057	0.1	0.26666666666666666	6.00	.01	300
i22	1.2500000000000142	0.1	0.3333333333333333	6.00	.01	300
i22	1.3500000000000085	0.1	0.4	6.00	.01	300
i22	1.450000000000017	0.1	0.4666666666666667	6.00	.01	300
i22	1.5500000000000114	0.1	0.5333333333333333	6.00	.01	300
i22	1.65000000000002	0.1	0.6	6.00	.01	300
i22	1.7500000000000142	0.1	0.6666666666666666	6.00	.01	300
i22	1.8500000000000227	0.1	0.7333333333333333	6.00	.01	300
i22	1.950000000000017	0.1	0.8	6.00	.01	300
i22	2.0500000000000256	0.1	0.8666666666666667	6.00	.01	300
i22	2.15000000000002	0.1	0.9333333333333333	6.00	.01	300
i22	2.2500000000000284	0.1	1	6.00	.01	300
i22	2.3500000000000227	0.1	1	6.00	.01	300
i22	2.4500000000000313	0.1	0.9166666666666666	6.00	.01	300
i22	2.5500000000000256	0.1	0.8333333333333334	6.00	.01	300
i22	2.650000000000034	0.1	0.75	6.00	.01	300
i22	2.7500000000000284	0.1	0.6666666666666667	6.00	.01	300
i22	2.850000000000037	0.1	0.5833333333333333	6.00	.01	300
i22	2.9500000000000313	0.1	0.5	6.00	.01	300
i22	3.05000000000004	0.1	0.41666666666666663	6.00	.01	300
i22	3.150000000000034	0.1	0.33333333333333337	6.00	.01	300
i22	3.2500000000000426	0.1	0.25	6.00	.01	300
i22	3.350000000000037	0.1	0.16666666666666663	6.00	.01	300
i22	3.4500000000000455	0.1	0.08333333333333337	6.00	.01	300
i22	3.55000000000004	0.05	0	6.00	.01	300
i22	3.600000000000037	0.05	0.14285714285714288	6.00	.01	300
i22	3.650000000000034	0.05	0.28571428571428575	6.00	.01	300
i22	3.7000000000000313	0.05	0.4285714285714286	6.00	.01	300
i22	3.7500000000000284	0.05	0.5714285714285715	6.00	.01	300
i22	3.8000000000000256	0.05	0.7142857142857143	6.00	.01	300
i22	3.8500000000000227	0.05	0.8571428571428572	6.00	.01	300
i22	3.90000000000002	0.05	1	6.00	.01	300
i22	3.950000000000017	0.1	0	6.00	.01	300
i22	4.050000000000011	0.1	0.06666666666666667	6.00	.01	300
i22	4.150000000000006	0.1	0.13333333333333333	6.00	.01	300
i22	4.25	0.1	0.2	6.00	.01	300
i22	4.349999999999994	0.1	0.26666666666666666	6.00	.01	300
i22	4.449999999999989	0.1	0.33333333333333337	6.00	.01	300
i22	4.549999999999983	0.1	0.4	6.00	.01	300
i22	4.649999999999977	0.1	0.4666666666666667	6.00	.01	300
i22	4.749999999999972	0.1	0.5333333333333333	6.00	.01	300
i22	4.849999999999966	0.1	0.6	6.00	.01	300
i22	4.94999999999996	0.1	0.6666666666666667	6.00	.01	300
i22	5.0499999999999545	0.1	0.7333333333333334	6.00	.01	300
i22	5.149999999999949	0.1	0.8	6.00	.01	300
i22	5.249999999999943	0.1	0.8666666666666667	6.00	.01	300
i22	5.3499999999999375	0.1	0.9333333333333333	6.00	.01	300
i22	5.449999999999932	0.1	1	6.00	.01	300
i22	5.549999999999926	0.1	1	6.00	.01	300
i22	5.64999999999992	0.1	0.9166666666666666	6.00	.01	300
i22	5.749999999999915	0.1	0.8333333333333334	6.00	.01	300
i22	5.849999999999909	0.1	0.75	6.00	.01	300
i22	5.949999999999903	0.1	0.6666666666666667	6.00	.01	300
i22	6.049999999999898	0.1	0.5833333333333334	6.00	.01	300
i22	6.149999999999892	0.1	0.5	6.00	.01	300
i22	6.249999999999886	0.1	0.41666666666666674	6.00	.01	300
i22	6.349999999999881	0.1	0.33333333333333337	6.00	.01	300
i22	6.449999999999875	0.1	0.25	6.00	.01	300
i22	6.549999999999869	0.1	0.16666666666666674	6.00	.01	300
i22	6.649999999999864	0.1	0.08333333333333337	6.00	.01	300
i22	6.749999999999858	0.05	0	6.00	.01	300
i22	6.799999999999855	0.05	0.14285714285714288	6.00	.01	300
i22	6.849999999999852	0.05	0.28571428571428575	6.00	.01	300
i22	6.899999999999849	0.05	0.4285714285714286	6.00	.01	300
i22	6.9499999999998465	0.05	0.5714285714285715	6.00	.01	300
i22	6.999999999999844	0.05	0.7142857142857143	6.00	.01	300
i22	7.049999999999841	0.05	0.8571428571428572	6.00	.01	300
i22	7.099999999999838	0.05	1	6.00	.01	300
i22	7.149999999999835	0.1	0	6.00	.01	300
i22	7.2499999999998295	0.1	0.06666666666666667	6.00	.01	300
i22	7.349999999999824	0.1	0.13333333333333333	6.00	.01	300
i22	7.449999999999818	0.1	0.2	6.00	.01	300
i22	7.549999999999812	0.1	0.26666666666666666	6.00	.01	300
i22	7.649999999999807	0.1	0.33333333333333337	6.00	.01	300
i22	7.749999999999801	0.1	0.4	6.00	.01	300
i22	7.849999999999795	0.1	0.4666666666666667	6.00	.01	300
i22	7.94999999999979	0.1	0.5333333333333333	6.00	.01	300
i22	8.049999999999784	0.1	0.6	6.00	.01	300
i22	8.149999999999778	0.1	0.6666666666666667	6.00	.01	300
i22	8.249999999999773	0.1	0.7333333333333334	6.00	.01	300
i22	8.349999999999767	0.1	0.8	6.00	.01	300
i22	8.449999999999761	0.1	0.8666666666666667	6.00	.01	300
i22	8.549999999999756	0.1	0.9333333333333333	6.00	.01	300
i22	8.64999999999975	0.1	1	6.00	.01	300
i22	8.749999999999744	0.1	1	6.00	.01	300
i22	8.849999999999739	0.1	0.9166666666666666	6.00	.01	300
i22	8.949999999999733	0.1	0.8333333333333334	6.00	.01	300
i22	9.049999999999727	0.1	0.75	6.00	.01	300
i22	9.149999999999721	0.1	0.6666666666666667	6.00	.01	300
i22	9.249999999999716	0.1	0.5833333333333334	6.00	.01	300
i22	9.34999999999971	0.1	0.5	6.00	.01	300
i22	9.449999999999704	0.1	0.41666666666666674	6.00	.01	300
i22	9.549999999999699	0.1	0.33333333333333337	6.00	.01	300
i22	9.649999999999693	0.1	0.25	6.00	.01	300
i22	9.749999999999687	0.1	0.16666666666666674	6.00	.01	300
i22	9.849999999999682	0.1	0.08333333333333337	6.00	.01	300
i22	9.949999999999676	0.05	0	6.00	.01	300
i22	9.999999999999673	0.05	0.14285714285714288	6.00	.01	300
i22	10.04999999999967	0.05	0.28571428571428575	6.00	.01	300
i22	10.099999999999667	0.05	0.4285714285714286	6.00	.01	300
i22	10.149999999999665	0.05	0.5714285714285715	6.00	.01	300
i22	10.199999999999662	0.05	0.7142857142857143	6.00	.01	300
i22	10.249999999999659	0.05	0.8571428571428572	6.00	.01	300
i22	10.299999999999656	0.05	1	6.00	.01	300
i22	10.349999999999653	0.1	0	6.00	.01	300
i22	10.449999999999648	0.1	0.06666666666666667	6.00	.01	300
i22	10.549999999999642	0.1	0.13333333333333333	6.00	.01	300
i22	10.649999999999636	0.1	0.2	6.00	.01	300
i22	10.74999999999963	0.1	0.26666666666666666	6.00	.01	300
i22	10.849999999999625	0.1	0.33333333333333337	6.00	.01	300
i22	10.94999999999962	0.1	0.4	6.00	.01	300
i22	11.049999999999613	0.1	0.4666666666666667	6.00	.01	300
i22	11.149999999999608	0.1	0.5333333333333333	6.00	.01	300
i22	11.249999999999602	0.1	0.6	6.00	.01	300
i22	11.349999999999596	0.1	0.6666666666666667	6.00	.01	300
i22	11.44999999999959	0.1	0.7333333333333334	6.00	.01	300
i22	11.549999999999585	0.1	0.8	6.00	.01	300
i22	11.64999999999958	0.1	0.8666666666666667	6.00	.01	300
i22	11.749999999999574	0.1	0.9333333333333333	6.00	.01	300
i22	11.849999999999568	0.1	1	6.00	.01	300
i22	11.949999999999562	0.1	1	6.00	.01	300
i22	12.049999999999557	0.1	0.9166666666666666	6.00	.01	300
i22	12.149999999999551	0.1	0.8333333333333334	6.00	.01	300
i22	12.249999999999545	0.1	0.75	6.00	.01	300
i22	12.34999999999954	0.1	0.6666666666666667	6.00	.01	300
i22	12.449999999999534	0.1	0.5833333333333334	6.00	.01	300
i22	12.549999999999528	0.1	0.5	6.00	.01	300
i22	12.649999999999523	0.1	0.41666666666666674	6.00	.01	300
i22	12.749999999999517	0.1	0.33333333333333337	6.00	.01	300
i22	12.849999999999511	0.1	0.25	6.00	.01	300
i22	12.949999999999505	0.1	0.16666666666666674	6.00	.01	300
i22	13.0499999999995	0.1	0.08333333333333337	6.00	.01	300
i22	13.149999999999494	0.05	0	6.00	.01	300
i22	13.199999999999491	0.05	0.14285714285714288	6.00	.01	300
i22	13.249999999999488	0.05	0.28571428571428575	6.00	.01	300
i22	13.299999999999486	0.05	0.4285714285714286	6.00	.01	300
i22	13.349999999999483	0.05	0.5714285714285715	6.00	.01	300
i22	13.39999999999948	0.05	0.7142857142857143	6.00	.01	300
i22	13.449999999999477	0.05	0.8571428571428572	6.00	.01	300
i22	13.499999999999474	0.05	1	6.00	.01	300
i22	13.549999999999471	0.1	0	6.00	.01	300
i22	13.649999999999466	0.1	0.06666666666666667	6.00	.01	300
i22	13.74999999999946	0.1	0.13333333333333333	6.00	.01	300
i22	13.849999999999454	0.1	0.2	6.00	.01	300
i22	13.949999999999449	0.1	0.26666666666666666	6.00	.01	300
i22	14.049999999999443	0.1	0.33333333333333337	6.00	.01	300
i22	14.149999999999437	0.1	0.4	6.00	.01	300
i22	14.249999999999432	0.1	0.4666666666666667	6.00	.01	300
i22	14.349999999999426	0.1	0.5333333333333333	6.00	.01	300
i22	14.44999999999942	0.1	0.6	6.00	.01	300
i22	14.549999999999415	0.1	0.6666666666666667	6.00	.01	300
i22	14.649999999999409	0.1	0.7333333333333334	6.00	.01	300
i22	14.749999999999403	0.1	0.8	6.00	.01	300
i22	14.849999999999397	0.1	0.8666666666666667	6.00	.01	300
i22	14.949999999999392	0.1	0.9333333333333333	6.00	.01	300
i22	15.049999999999386	0.1	1	6.00	.01	300
i22	15.14999999999938	0.1	1	6.00	.01	300
i22	15.249999999999375	0.1	0.9166666666666666	6.00	.01	300
i22	15.349999999999369	0.1	0.8333333333333334	6.00	.01	300
i22	15.449999999999363	0.1	0.75	6.00	.01	300
i22	15.549999999999358	0.1	0.6666666666666667	6.00	.01	300
i22	15.649999999999352	0.1	0.5833333333333334	6.00	.01	300
i22	15.749999999999346	0.1	0.5	6.00	.01	300
i22	15.84999999999934	0.1	0.41666666666666674	6.00	.01	300
i22	15.949999999999335	0.1	0.33333333333333337	6.00	.01	300
i22	16.04999999999933	0.1	0.25	6.00	.01	300
i22	16.149999999999324	0.1	0.16666666666666674	6.00	.01	300
i22	16.249999999999318	0.1	0.08333333333333337	6.00	.01	300
i22	16.349999999999312	0.05	0	6.00	.01	300
i22	16.39999999999931	0.05	0.14285714285714288	6.00	.01	300
i22	16.449999999999307	0.05	0.28571428571428575	6.00	.01	300
i22	16.499999999999304	0.05	0.4285714285714286	6.00	.01	300
i22	16.5499999999993	0.05	0.5714285714285715	6.00	.01	300
i22	16.599999999999298	0.05	0.7142857142857143	6.00	.01	300
i22	16.649999999999295	0.05	0.8571428571428572	6.00	.01	300
i22	16.699999999999292	0.05	1	6.00	.01	300
i22	16.74999999999929	0.1	0	6.00	.01	300
i22	16.849999999999284	0.1	0.06666666666666667	6.00	.01	300
i22	16.949999999999278	0.1	0.13333333333333333	6.00	.01	300
i22	17.049999999999272	0.1	0.2	6.00	.01	300
i22	17.149999999999267	0.1	0.26666666666666666	6.00	.01	300
i22	17.24999999999926	0.1	0.33333333333333337	6.00	.01	300
i22	17.349999999999255	0.1	0.4	6.00	.01	300
i22	17.44999999999925	0.1	0.4666666666666667	6.00	.01	300
i22	17.549999999999244	0.1	0.5333333333333333	6.00	.01	300
i22	17.64999999999924	0.1	0.6	6.00	.01	300
i22	17.749999999999233	0.1	0.6666666666666667	6.00	.01	300
i22	17.849999999999227	0.1	0.7333333333333334	6.00	.01	300
i22	17.94999999999922	0.1	0.8	6.00	.01	300
i22	18.049999999999216	0.1	0.8666666666666667	6.00	.01	300
i22	18.14999999999921	0.1	0.9333333333333333	6.00	.01	300
i22	18.249999999999204	0.1	1	6.00	.01	300
i22	18.3499999999992	0.1	1	6.00	.01	300
i22	18.449999999999193	0.1	0.9166666666666666	6.00	.01	300
i22	18.549999999999187	0.1	0.8333333333333334	6.00	.01	300
i22	18.64999999999918	0.1	0.75	6.00	.01	300
i22	18.749999999999176	0.1	0.6666666666666667	6.00	.01	300
i22	18.84999999999917	0.1	0.5833333333333334	6.00	.01	300
i22	18.949999999999164	0.1	0.5	6.00	.01	300
i22	19.04999999999916	0.1	0.41666666666666674	6.00	.01	300
i22	19.149999999999153	0.1	0.33333333333333337	6.00	.01	300
i22	19.249999999999147	0.1	0.25	6.00	.01	300
i22	19.34999999999914	0.1	0.16666666666666674	6.00	.01	300
i22	19.449999999999136	0.1	0.08333333333333337	6.00	.01	300
i22	19.54999999999913	0.05	0	6.00	.01	300
i22	19.599999999999127	0.05	0.14285714285714288	6.00	.01	300
i22	19.649999999999125	0.05	0.28571428571428575	6.00	.01	300
i22	19.69999999999912	0.05	0.4285714285714286	6.00	.01	300
i22	19.74999999999912	0.05	0.5714285714285715	6.00	.01	300
i22	19.799999999999116	0.05	0.7142857142857143	6.00	.01	300
i22	19.849999999999113	0.05	0.8571428571428572	6.00	.01	300
i22	19.89999999999911	0.05	1	6.00	.01	300
i22	19.949999999999108	0.1	0	6.00	.01	300
i22	20.049999999999102	0.1	0.06666666666666667	6.00	.01	300
i22	20.149999999999096	0.1	0.13333333333333333	6.00	.01	300
i22	20.24999999999909	0.1	0.2	6.00	.01	300
i22	20.349999999999085	0.1	0.26666666666666666	6.00	.01	300
i22	20.44999999999908	0.1	0.33333333333333337	6.00	.01	300
i22	20.549999999999073	0.1	0.4	6.00	.01	300
i22	20.649999999999068	0.1	0.4666666666666667	6.00	.01	300
i22	20.749999999999062	0.1	0.5333333333333333	6.00	.01	300
i22	20.849999999999056	0.1	0.6	6.00	.01	300
i22	20.94999999999905	0.1	0.6666666666666667	6.00	.01	300
i22	21.049999999999045	0.1	0.7333333333333334	6.00	.01	300
i22	21.14999999999904	0.1	0.8	6.00	.01	300
i22	21.249999999999034	0.1	0.8666666666666667	6.00	.01	300
i22	21.349999999999028	0.1	0.9333333333333333	6.00	.01	300
i22	21.449999999999022	0.1	1	6.00	.01	300
i22	21.549999999999017	0.1	1	6.00	.01	300
i22	21.64999999999901	0.1	0.9166666666666666	6.00	.01	300
i22	21.749999999999005	0.1	0.8333333333333334	6.00	.01	300
i22	21.849999999999	0.1	0.75	6.00	.01	300
i22	21.949999999998994	0.1	0.6666666666666667	6.00	.01	300
i22	22.049999999998988	0.1	0.5833333333333334	6.00	.01	300
i22	22.149999999998983	0.1	0.5	6.00	.01	300
i22	22.249999999998977	0.1	0.41666666666666674	6.00	.01	300
i22	22.34999999999897	0.1	0.33333333333333337	6.00	.01	300
i22	22.449999999998965	0.1	0.25	6.00	.01	300
i22	22.54999999999896	0.1	0.16666666666666674	6.00	.01	300
i22	22.649999999998954	0.1	0.08333333333333337	6.00	.01	300
i22	22.74999999999895	0.05	0	6.00	.01	300
i22	22.799999999998946	0.05	0.14285714285714288	6.00	.01	300
i22	22.849999999998943	0.05	0.28571428571428575	6.00	.01	300
i22	22.89999999999894	0.05	0.4285714285714286	6.00	.01	300
i22	22.949999999998937	0.05	0.5714285714285715	6.00	.01	300
i22	22.999999999998934	0.05	0.7142857142857143	6.00	.01	300
i22	23.04999999999893	0.05	0.8571428571428572	6.00	.01	300
i22	23.09999999999893	0.05	1	6.00	.01	300
i23	23.150000000000006	1.6	.5	1.00	.05	.0
i99	0.05000000000000426	27.05	2.5

e
</CsScore>
</CsoundSynthesizer>
