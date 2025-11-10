set -e

function make_necessary_dirs {
    mkdir -p data
    cd data
    mkdir -p ascii
    mkdir -p demo
    mkdir -p eff00
    mkdir -p end
    mkdir -p etama
    mkdir -p face
    mkdir -p front
    mkdir -p image
    mkdir -p player00
    mkdir -p result
    mkdir -p stg1bg
    mkdir -p stgenm
    mkdir -p title
    mkdir -p wav
}

GAME_LOCATION=$1

if [ -z "$GAME_LOCATION" ]; then
    echo "Usage: $0 <game_location>"
    exit 1
fi

cd $GAME_LOCATION
make_necessary_dirs

for file in $GAME_LOCATION*.DAT; do
    printf "Processing $file..."
    mkdir -p thdat_output
    thdat -x 6 "$file" -C $GAME_LOCATION/data/thdat_output > /dev/null
    printf " Done!\n"
done;

echo "Moving files..."

cd $GAME_LOCATION/data
mv thdat_output/ascii*.png ./ascii
mv thdat_output/demo00.rpy ./demo
mv thdat_output/eff*.png ./eff00
mv thdat_output/end*.jpg ./end
mv thdat_output/staff00.jpg ./end
mv thdat_output/staff*.png ./end
mv thdat_output/etama*.png ./etama
mv thdat_output/face*.png ./face
mv thdat_output/front*.png ./front
mv thdat_output/loading.png ./image
mv thdat_output/player*.png ./player00
mv thdat_output/music.jpg ./result
mv thdat_output/music*.png ./result
mv thdat_output/result.jpg ./result
mv thdat_output/result*.png ./result
mv thdat_output/stg*bg*.png ./stg1bg
mv thdat_output/stg*enm*.png ./stgenm
mv thdat_output/replay*.png ./title
mv thdat_output/select00.jpg ./title
mv thdat_output/select*.png ./title
mv thdat_output/slpl*.png ./title
mv thdat_output/th06logo.jpg ./title
mv thdat_output/title00.jpg ./title
mv thdat_output/title*.png ./title
mv thdat_output/*.wav ./wav

mv thdat_output/*.pos ../bgm/
# FIXME! Midi support currently doesn't exist in the portable build, and it probably won't in the remaster. I don't think we need these.
rm thdat_output/*.mid

mv thdat_output/*.* ./
rm -rf thdat_output

echo "Done!"
