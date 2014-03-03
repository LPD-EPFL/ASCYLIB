
MAKE=make

unm=$(uname -n);
if [ $unm = "ol-collab1" ];
then
    MAKE=gmake
fi;

source scripts/lat_distribution.config
source scripts/executables.config

if [ $# -eq 0 ];		# pass any param to avoid compilation
then
    LATENCY=2 INIT=one GRANULARITY=GLOBAL_LOCK $MAKE -k tas
    LATENCY=2 INIT=one $MAKE -k tas
    LATENCY=2 INIT=one $MAKE -k lockfree
fi

echo ${lf_ll}
./scripts/lat_distribution.sh ${unm}_lat_lf_ll ${lf_ll} -d1000 -f1024 -v1024
echo ${lb_ll}
./scripts/lat_distribution.sh ${unm}_lat_lb_ll ${lb_ll} -d1000 -f1024 -v1024
echo ${lf_ht}
./scripts/lat_distribution.sh ${unm}_lat_lf_ht ${lf_ht} -d1000 -f1024 -v1024
echo ${lb_ht}
./scripts/lat_distribution.sh ${unm}_lat_lb_ht ${lb_ht} -d1000 -f1024 -v1024
echo ${lf_sl}
./scripts/lat_distribution.sh ${unm}_lat_lf_sl ${lf_sl} -d1000 -f1024 -v1024
echo ${lb_sl}
./scripts/lat_distribution.sh ${unm}_lat_lb_sl ${lb_sl} -d1000 -f1024 -v1024
echo ${lf_bst}
./scripts/lat_distribution.sh ${unm}_lat_lf_bst ${lf_bst} -d1000 -f1024 -v1024
echo ${lb_bst}
./scripts/lat_distribution.sh ${unm}_lat_lb_bst ${lb_bst} -d1000 -f1024 -v1024

