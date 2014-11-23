#ifndef		WC_TYPES_HPP
#define		WC_TYPES_HPP

#include	<list>
#include	<thread>
#include	<string>
#include	<vector>
#include	<cmath>

typedef	std::string				wcURL;
typedef	const char*				wcPath;
typedef	char*					wcBuffer;
typedef	std::string				wcString;
typedef	std::list<wcString>		wcStringCell;
typedef	unsigned long			wcSize;
typedef	unsigned long			wcPos;
typedef	unsigned int 			wcFlags;
typedef std::thread				wcThread;
typedef float					wcFloat;
typedef	std::pair<wcPos,wcPos>	wcRange;
typedef	std::vector<wcRange>	wcRangeVector;



class wcMetaVector
{
public:
	wcFloat						p_density;
	wcFloat						p_quality;
	wcFloat						p_relevence;

	wcFloat operator*=( const wcMetaVector& other );
	void	operator*=( const wcFloat& scale );
	void	operator/=( const wcFloat& scale );
	void	operator+=( const wcMetaVector& other );
	void	operator-=( const wcMetaVector& other );

	wcMetaVector();
	~wcMetaVector();
};

#endif
