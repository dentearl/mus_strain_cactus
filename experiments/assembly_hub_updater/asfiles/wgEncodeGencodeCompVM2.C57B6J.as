table wgEncodeGencodeCompVM2C57B6J
"C57B6J genes with additional fields commonName"
(
string	chrom;	"Reference sequence chromosome or scaffold"
uint	chromStart;	"Start position of feature on chromosome"
uint	chromEnd;	"End position of feature on chromosome"
string	name;	"Name of gene"
uint	score;	"Score"
char[1]	strand;	"+ or - for strand"
uint	thickStart;	"Coding region start"
uint	thickEnd;	"Coding region end"
uint	reserved;	"RGB value"
int	blockCount;	"Number of blocks"
int[blockCount]	blockSizes;	"A comma-separated list of block sizes"
int[blockCount]	chromStarts;	"A comma-separated list of block starts"
string	commonName;	"Gene common name"
)
