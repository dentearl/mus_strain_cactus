#!/bin/bash -e
# update the mouse browser.
# not robust in the slightest.
set -o errexit
set -o pipefail
set -o nounset

if [ $# -eq 0 ]; then
  echo "Error, you must specify the release number as the first argument, either 1302, 1405, 1411, or 1409"
  exit 1
fi
release=$1

MUS_STRAIN_CACTUS_DIR=$(readlink -f $(dirname $0)/../..)
PROGRESSIVE_CACTUS_DIR=/cluster/home/jcarmstr/progressiveCactus-msca
MOUSE_HUBS_DIR=$MUS_STRAIN_CACTUS_DIR/hubsToTestIndelAnnot
HAL_FILE=/hive/users/jcarmstr/msca/$release/$release.hal

if [ "$release" != "1302" ] && [ "$release" != "1405" ] && [ "$release" != "1409" ] && [ "$release" != "1411" ]; then
  echo "Release must be either 1302, 1405, 1409, or 1411."
  exit
fi

# Rattus is put at the front of the list (out of alphabetical order) because
# it takes an extremely long time to run. I'd rather get it done first than
# be sitting and waiting for it to finish at the very end.
if [ "$release" == "1302" ]; then
  genomes="Rattus Anc0 Anc1 Anc2 Anc3 Anc4 Anc5 Anc6 Anc7 Anc8 AKRJ BALBcJ C3HHeJ C57B6J C57B6NJ CBAJ DBA2J FVBNJ NZOHlLtJ"
else
  genomes="Rattus Anc00 Anc01 Anc02 Anc03 Anc04 Anc05 Anc06 Anc07 Anc08 Anc09 Anc10 Anc11 Anc12 Anc13 Anc14 Anc15 Anc16 129S1 AJ AKRJ BALBcJ CASTEiJ C3HHeJ C57B6J C57B6NJ CBAJ DBA2J FVBNJ LPJ NODShiLtJ NZOHlLtJ PWKPhJ SPRETEiJ WSBEiJ"
fi

# python
PYTHONPATH=$PYTHONPATH:$PROGRESSIVE_CACTUS_DIR/submodules
PATH=$PATH:$PROGRESSIVE_CACTUS_DIR/submodules/hal/bin

echo "# Creating a hub for release $release"
mkdir -p $MOUSE_HUBS_DIR/mouseBrowser_$release

echo '# Updating bed files'
mkdir -p $MOUSE_HUBS_DIR/bedDirs_$release
$MUS_STRAIN_CACTUS_DIR/pipeline/src/result2bedDirs.py --outDir $MOUSE_HUBS_DIR/bedDirs_$release $MUS_STRAIN_CACTUS_DIR/pipeline/results_$release/metaFilter.*

if [ ! -d "$MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene" ]; then
  # this will actually be considered a "custom" bed but whatever. I aint got time to make this perfect.
  echo '# Extracting ensGene for Rattus'
  mkdir -p $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus
  hgsql -h hgwdev -e "select * from ensGene" rn5 | cut -f 2- > $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/rn5.ensGene.gp
  python $MUS_STRAIN_CACTUS_DIR/src/ngan_scripts/gp2bed.py $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/rn5.ensGene.gp $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed.tmp && rm $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/rn5.ensGene.gp
  perl -ple 's/chr//' < $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed.tmp > $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed && rm $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed.tmp
  if [ "$release" == "1409" ]; then
      # Rattus in this release had .1 after some scaffolds, which was renamed to _1
      sed -i -E 's/(AABR[0-9]*)/\1_1/g' $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed
      sed -i -E 's/(JH[0-9]*)/\1_1/g' $MOUSE_HUBS_DIR/bedDirs_$release/ratEnsGene/Rattus/Rattus.ensGene.bed
  fi
fi

echo '# Checking for custom beds'
extra_beds=""
for d in $MOUSE_HUBS_DIR/bedDirs_$release/*/; do
  _d=$(basename $d)
  if [[ $_d != "input_geneCheck" ]] && [[ $_d != "metaFilter" ]] && [[ $_d != "input_geneCheck_details" ]] && [[ $_d != "metaFilter_details" ]]; then
    extra_beds="$extra_beds,$MOUSE_HUBS_DIR/bedDirs_$release/$_d"
  fi
done

echo '# Bringing over original input gene-check beds'
mkdir -p $MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck/C57B6J
mkdir -p $MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck_details/C57B6J
cp /hive/groups/recon/projs/mus_strain_cactus/data/gene_check/wgEncodeGencodeBasicVM2.coding.gene-check.bed $MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck/C57B6J/C57B6J.wgEncodeGencodeBasicVM2.coding.gene-check.bed
cp /hive/groups/recon/projs/mus_strain_cactus/data/gene_check/wgEncodeGencodeBasicVM2.coding.gene-check-details.bed $MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck_details/C57B6J/C57B6J.wgEncodeGencodeBasicVM2.coding.gene-check-details.bed

echo '# Removing old symlinks'
for d in refGene knownGene wgEncodeGencodeCompVM2 wgEncodeGencodeCompVM2_CDS; do
  rm -f /cluster/home/dearl/msca/myMouseBrowser/browser_$release/liftoverbed/$d/$d
done

if [ ! -d "$MOUSE_HUBS_DIR/bigBedDirs_$release/refGene" ]; then
  echo "# Extracting refGene, knownGene, wgEncodeGencodeComp data."
  mkdir -p liftover_$release
  hgsql -h hgwdev -e "select * from refGene" mm10 | cut -f 2- > ./liftover_$release/refGene.gp &
  hgsql -h hgwdev -e "select * from knownGene" mm10  | cut -f 2- > ./liftover_$release/knownGene.gp &
  hgsql -h hgwdev -e "select * from wgEncodeGencodeCompVM2" mm10  | cut -f 2- > ./liftover_$release/wgEncodeGencodeCompVM2.gp &
  wait
  python $MUS_STRAIN_CACTUS_DIR/src/ngan_scripts/gp2bed.py liftover_$release/refGene.gp liftover_$release/refGene.bed &
  python $MUS_STRAIN_CACTUS_DIR/src/ngan_scripts/knowngene2bed.py liftover_$release/knownGene.gp liftover_$release/knownGene.bed &
  python $MUS_STRAIN_CACTUS_DIR/src/ngan_scripts/gp2bed.py liftover_$release/wgEncodeGencodeCompVM2.gp liftover_$release/wgEncodeGencodeCompVM2.bed &
  wait
  echo "# Performing liftover (this takes several hours)"
  for g in $genomes; do
    for b in refGene knownGene wgEncodeGencodeCompVM2; do
      ##########
      # This block holds the background process count in check
      while [ $(jobs | wc -l) -gt 8 ]; do
        sleep 5
      done
      /hive/groups/recon/projs/mus_strain_cactus/src/progressiveCactus/submodules/hal/bin/halLiftover $HAL_FILE C57B6J liftover_$release/$b.bed $g liftover_$release/$b.$g.bed_tmp --keepExtra --outBedVersion 13 --tab &> liftover_$release/_log.liftover.$b.$g &
    done
  done
  wait
  echo "# Sorting lift over beds"
  for g in $genomes; do
    for b in refGene knownGene wgEncodeGencodeCompVM2; do
      bedSort liftover_$release/$b.$g.bed_tmp liftover_$release/$b.$g.bed.srt &
    done
    wait
    for b in refGene knownGene wgEncodeGencodeCompVM2; do
      rm liftover_$release/$b.$g.bed_tmp
      mv liftover_$release/$b.$g.bed.srt liftover_$release/$b.$g.bed
    done
  done
  echo "# Creating big beds from liftover"
  for b in refGene knownGene wgEncodeGencodeCompVM2; do
    mkdir -p $MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J
    cp $MUS_STRAIN_CACTUS_DIR/experiments/assembly_hub_updater/asfiles/$b.C57B6J.as $MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J/C57B6J.as
  done
  for g in $genomes; do
    for b in refGene knownGene wgEncodeGencodeCompVM2; do
      # these will lack thick-thin distinction
      bedToBigBed -tab -type=bed12+1 -extraIndex=name,commonName -as=$MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J/C57B6J.as liftover_$release/$b.$g.bed /hive/groups/recon/projs/mus_strain_cactus/data/assembly_rel_$release/$g.sizes $MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J/$g.bb &> _log.bedToBigBed.$b.$g  &
    done
    wait
  done
  echo "# Placing the original C57B6J big beds into the browser"
  for b in refGene knownGene wgEncodeGencodeCompVM2; do
    # we do this to get the thick-thin distinction
    bedSort liftover_$release/$b.bed liftover_$release/$b.bed.srt
    bedToBigBed -tab -type=bed12+1 -extraIndex=name,commonName -as=$MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J/C57B6J.as liftover_$release/$b.bed.srt /hive/groups/recon/projs/mus_strain_cactus/data/assembly_rel_$release/C57B6J.sizes $MOUSE_HUBS_DIR/bigBedDirs_$release/$b/C57B6J/C57B6J.bb &> _log.bedToBigBed.$b.C57B6J  &
  done
  wait
fi

if [ ! -f $MOUSE_HUBS_DIR/mouseBrowser_$release/lod.txt ]; then
  echo '# Generating hal LOD files (this can take a day or so)'
  pushd $PROGRESSIVE_CACTUS_DIR/submodules/ > /dev/null
  OLD_PATH=$PATH
  PATH=$PATH:$PROGRESSIVE_CACTUS_DIR/submodules/hal/bin
  $PROGRESSIVE_CACTUS_DIR/submodules/hal/bin/halLodInterpolate.py $HAL_FILE $MOUSE_HUBS_DIR/mouseBrowser_$release/lod.txt.tmp --outHalDir $MOUSE_HUBS_DIR/mouseBrowser_$release/lod --numProc 10 --inMemory
  mv $MOUSE_HUBS_DIR/mouseBrowser_$release/lod.txt.tmp $MOUSE_HUBS_DIR/mouseBrowser_$release/lod.txt
  PATH=$OLD_PATH
  popd > /dev/null
fi

echo '# Removing old job tree'
rm -rf jt_assembly_hub_$release

echo '# Launching hal2assemblyHub.py'
# create an assembly hub
# --finalBigBedDirs are for things we already liftedover on our own
# --bedDirs we use for things we dont want to liftover
$PROGRESSIVE_CACTUS_DIR/submodules/hal/bin/hal2assemblyHub.py $HAL_FILE \
browser_$release \
--hub=msca_$release --shortLabel=MouseStrain_$release \
--longLabel="Mouse Strain Comparative Assembly Hub release $release" --email='benedict@soe.ucsc.edu' \
--url="http://hgwdev.sdsc.edu/~benedict/mouseBrowser_$release" \
--twobitdir=/hive/groups/recon/projs/mus_strain_cactus/data/assembly_rel_$release \
--lod --lodTxtFile=$MOUSE_HUBS_DIR/mouseBrowser_$release/lod.txt \
--lodDir=$MOUSE_HUBS_DIR/mouseBrowser_$release/lod \
--finalBigBedDirs=$MOUSE_HUBS_DIR/bigBedDirs_$release/refGene,$MOUSE_HUBS_DIR/bigBedDirs_$release/knownGene,$MOUSE_HUBS_DIR/bigBedDirs_$release/wgEncodeGencodeCompVM2 \
--bedDirs=$MOUSE_HUBS_DIR/bedDirs_$release/metaFilter,$MOUSE_HUBS_DIR/bedDirs_$release/metaFilter_details,$MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck,$MOUSE_HUBS_DIR/bedDirs_$release/input_geneCheck_details$extra_beds \
--tabBed \
--jobTree=./jt_assembly_hub_$release \
--batchSystem=singleMachine \
--stats \
--maxThreads=16 \
--logInfo \
--noBedLiftover

echo '# Correcting genomes.txt'
perl -ple "s/orderKey 4800/orderKey 4800\ndescription rel_$release/" -i browser_$release/genomes.txt

echo '# Copying genome files.'
# copy over genome files
for g in $genomes documentation; do
  mkdir -p $MOUSE_HUBS_DIR/mouseBrowser_$release/$g/
  cp -r browser_$release/$g/* $MOUSE_HUBS_DIR/mouseBrowser_$release/$g/ &
done
wait

if [ ! -f ../mouseBrowser_$release/msca.hal ]; then
  echo '# Copying main hal file.'
  cp $HAL_FILE $MOUSE_HUBS_DIR/mouseBrowser_$release/$(basename $HAL_FILE)
fi

echo '# Copying bed files.'
# copy over bed files
for d in metaFilter metaFilter_details input_geneCheck input_geneCheck_details; do
  mkdir -p $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$d;
  cp -r browser_$release/liftoverbed/$d/* $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$d/ &
done
wait
echo '# Copying custom bed files.'
for d in $MOUSE_HUBS_DIR/bedDirs_$release/*/; do
  _d=$(basename $d)
  if [[ $_d != "input_geneCheck" ]] && [[ $_d != "metaFilter" ]] && [[ $_d != "input_geneCheck_details" ]] && [[ $_d != "metaFilter_details" ]]; then
    mkdir -p $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$_d;
    cp -r browser_$release/liftoverbed/$_d/* $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$_d/ &
  fi
done
wait
for b in refGene knownGene wgEncodeGencodeCompVM2; do
  mkdir -p $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$b/C57B6J
  for f in browser_$release/liftoverbed/$b/C57B6J/*; do
    cp $f $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$b/C57B6J/ &
  done
done
wait
for b in refGene knownGene wgEncodeGencodeCompVM2; do
  cp browser_$release/liftoverbed/$b/C57B6J/C57B6J.as $MOUSE_HUBS_DIR/mouseBrowser_$release/liftoverbed/$b/C57B6J/ &
done
wait

echo '# Copying incidental files.'
# copy over files
for f in genomes.txt groups.txt hub.txt hubTreeModified.nw haltree.nw; do
  cp browser_$release/$f $MOUSE_HUBS_DIR/mouseBrowser_$release/ &
done
wait

echo '# Replacing 2bit symlinks with actual 2bit files.'
# replace 2bit symlinks with actual 2bit files
for g in $genomes; do
  rm -f $MOUSE_HUBS_DIR/mouseBrowser_$release/$g/$g.2bit;
  cp /hive/groups/recon/projs/mus_strain_cactus/data/assembly_rel_$release/$g.2bit $MOUSE_HUBS_DIR/mouseBrowser_$release/$g/$g.2bit &
done
wait

echo '# Fixing perimissions.'
# fix permissions
chmod -R g+w $MOUSE_HUBS_DIR/mouseBrowser_$release/* &> chmod_$release.log

echo '# Done.'
