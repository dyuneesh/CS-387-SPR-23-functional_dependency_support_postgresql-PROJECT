

export POSTGRES_SRCDIR=/home/bhuvan/dbms/PROJ/postgresql
export POSTGRES_INSTALLDIR=/home/bhuvan/dbms/PROJ/postgresql/install
cd ${POSTGRES_SRCDIR}
./configure --prefix=${POSTGRES_INSTALLDIR} --enable-debug
export enable_debug=yes
make | tee gmake.out
make install | tee gmake_install.out
export LD_LIBRARY_PATH=${POSTGRES_INSTALLDIR}/lib:${LD_LIBRARY_PATH}
export PATH=${POSTGRES_INSTALLDIR}/bin:${PATH}
export PGDATA=${POSTGRES_INSTALLDIR}/data


install/bin/initdb -D ${PGDATA}
install/bin/pg_ctl -D $PGDATA -l logfile start
install/bin/createdb -p 5432 test
# install/bin/psql -p 5432 test
install/bin/pg_ctl stop

#SINGLE USER MODE
# postgres --single -D ${PGDATA} test

#CLEANUP
# make distclean