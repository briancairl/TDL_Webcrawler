#ifndef WC_DEF_HPP
#define WC_DEF_HPP

	#define		wcMASK(n)			(1<<n)
	#define		wcSET(flags,n)		flags|=(1<<n)
	#define		wcCLEAR(flags,n)	flags&=~(1<<n)
	#define		wcIS_SET(flags,n)	(flags&(1<<n))
	#define		wcIS_CLEAR(flags,n)	(!wcIS_SET(flags,n))


	#define		wcVALCHAR(c)		(	((c>47)&&(c<58) )	\
									||	((c>64)&&(c<91) )	\
									||	((c>96)&&(c<123))	\
									)


	#define		wcDELETE(ptr)		if(ptr) { delete ptr; ptr = NULL; }
#endif