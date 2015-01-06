ROOT = ../..

JKDIR = ${HOME}/kent/src
MACHTYPE = x86_64
JKLIBDIR = ${JKDIR}/lib/${MACHTYPE}

MYSQLLIBS = $(shell mysql_config --libs)
SAMTABIXLIB=/hive/data/outside/samtabix/${MACHTYPE}/libsamtabix.a

INCL = -I${JKDIR}/inc -I${JKDIR}/hg/inc 
LIBS = ${JKLIBDIR}/jkhgap.a ${JKLIBDIR}/jkweb.a ${SAMTABIXLIB} -lz ${MYSQLLIBS} -lssl -lcrypto

BINDIR = ${ROOT}/bin
OBJDIR = ${ROOT}/objs

GENE_CHECK = ${BINDIR}/gene-check
PROGS = ${GENE_CHECK}

SRCBASENAMES = $(basename $(wildcard *.cc))

OBJS = ${SRCBASENAMES:%=${OBJDIR}/%.o}

all: ${PROGS}

${GENE_CHECK}: ${OBJS}
	@mkdir -p ${BINDIR}
	${CXX} ${CXXFLAGS} -o ${GENE_CHECK} ${OBJS} ${LIBS}

${OBJDIR}/%.o: %.cc
	@mkdir -p ${OBJDIR}
	${CXX} ${CXXFLAGS} ${INCL} -c -o $@ $<

clean:
	rm -f ${OBJS} ${PROGS}

