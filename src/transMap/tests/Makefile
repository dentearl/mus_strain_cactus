
# this requires an existing MAF to run the tests, might need to edit these
halBin = /cluster/home/markd/compbio/gencode/mus_strain_cactus/build/hal/bin
srcGencodeTbl = wgEncodeGencodeBasicVM3
srcHgDb= mm10
srcOrg = C57B6J
destOrg =  C57B6NJ
hal = /hive/groups/recon/projs/mus_strain_cactus/data/assembly_rel_1405/msca.hal

# extraced gencode
srcGp = output/gencode.C57B6NJ.gp

export PATH:=${halBin}:${PATH}

test: testTransmapGp

# halTransMapGenePred calls halTransMapPsl, so both are tested
testTransmapGp: ${srcGp}
	../bin/halTransMapGenePred --localNearBestFilter=0.001 ${hal} ${srcGp} ${srcOrg} ${destOrg} output/$@.psl output/$@.gp output/$@.cds


# get some test source genes, converting chr19 -> 19
${srcGp}: mkdirs
	hgsql -Ne 'select * from ${srcGencodeTbl} where chrom="chr19" limit 100' ${srcHgDb} | cut -f 2- | tawk '{$$2=substr($$2, 4);print $0}' >$@.tmp
	mv -f $@.tmp $@

mkdirs:
	@mkdir -p output

clean:
	rm -rf output
