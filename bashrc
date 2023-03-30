export POSTGRES_INSTALLDIR=/home/dyuneesh/Desktop/6th_sem/dibs_317_387/project/CS-387-SPR-23-functional_dependency_support_postgresql-PROJECT/postgresql/install/
export POSTGRES_SRCDIR=/home/dyuneesh/Desktop/6th_sem/dibs_317_387/project/CS-387-SPR-23-functional_dependency_support_postgresql-PROJECT/postgresql/

export LD_LIBRARY_PATH=${POSTGRES_INSTALLDIR}/lib:${LD_LIBRARY_PATH}
export PATH=${POSTGRES_INSTALLDIR}/bin:${PATH}
export PGDATA=${POSTGRES_INSTALLDIR}/data