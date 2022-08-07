while getopts s: flag;
do
    case "${flag}" in
        s) stage="${OPTARG}";;
    esac
done

if [ -z $stage ];
then
    printf "Usage: test.sh -s stage_number\n"
    printf "Stage number must be specified\n"
else
    dir="tests/stage${stage}"
    filename="${dir}/test_stage${stage}.sh"

    ./${filename}
fi

