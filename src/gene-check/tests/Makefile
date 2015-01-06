ROOT = ../../..
BINDIR = ${ROOT}/bin

HG15_NIB_DIR = /cluster/data/hg15/nib
HG16_NIB_DIR = /cluster/data/hg16/nib
HG17_NIB_DIR = /cluster/data/hg17/nib

GENE_CHECK = ${BINDIR}/gene-check


# note nibs not in test dir
test:: test1 test2 test3 test4 test5 test6 test7 test8

# basic test
test1:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG15_NIB_DIR} \
		--details-out=output/$@/$@.details \
		input/refGene.hg15.part.gp \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

# test with CDS
test2:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG15_NIB_DIR} --mrna-fasta \
		--cds-file=input/refGene.hg15.part.cds \
		input/refGene.hg15.part.fa \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@


# test with no CDS
test3:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG16_NIB_DIR} \
		--details-out=output/$@/$@.details \
	        input/noCDS.hg16.gp \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@


# test with CDS and frame
test4:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG15_NIB_DIR} \
		--cds-file=input/refGene.hg15.part.cds \
		--details-out=output/$@/$@.details \
		input/refGene.hg15.frame.gp \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

# Test for checking only CDS splice sites.  utrSplice.hg17.gp false reported
# errors in spliceing when start or stop codon was on a splice boundry.
test5:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG17_NIB_DIR} --cds-only \
		input/utrSplice.hg17.gp \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

# gene has a 2 base exon and a bogus splice site that was not detected.
test6:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG17_NIB_DIR} \
		--details-out=output/$@/$@.details \
		input/regress.gp output/$@/$@.stat \
		 >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

# frame-shifted gene
test7:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG17_NIB_DIR} \
		--details-out=output/$@/$@.details \
		input/frameShift.hg17.gp output/$@/$@.stat \
		 >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

# no checking of cds features
test8:
	@mkdir -p output/$@
	${GENE_CHECK} --genome-seqs=${HG15_NIB_DIR} --no-cds-check \
		--details-out=output/$@/$@.details \
		input/refGene.hg15.part.gp \
		output/$@/$@.stat >output/$@/$@.out 2>&1
	diff --exclude=.svn -r expected/$@ output/$@

clean::
	rm -rf output
