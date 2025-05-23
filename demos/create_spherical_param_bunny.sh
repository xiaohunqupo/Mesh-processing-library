#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"
source bin/_initdemos.sh


echo 'Compute a mapping from the flat octahedron to the sphere that is optimized for inverse stretch.'
SphereSample -domain octaflat -scheme domain -egrid 128 -keys domaincorner,imageuv -mesh_sphere | Filtermesh -gmerge -removekey Ovi -cornermerge -renamekey v domaincorner global -renamekey vc imageuv uv -assign_normals -removekey sharp | bin/meshtopm.sh -minqem -qemvolume 0 -dihallow -affectpq 3 -vsgeom -keepglobalv 1 -norfac 0 -trishapeafac 1e1 -strict_sharp 2 >data/v_sharp.pm

SphereParam data/v_sharp.pm -base coord -flatten_to_x0 -fix_base -optimize_inverse -respect_sharp_edges -keep_uv 1 >data/v.m

FilterPM data/v_sharp.pm -finest -truncate_prior | SphereParam -mesh_for_base data/v.m -fix_base -optimize_inverse -keep_uv 1 | Filtermesh -genus -removekey normal -removekey wid -cornermerge >data/octaflat_eg128.uv.sphparam.m

rm -f data/v_sharp.pm data/v.m


echo 'From the original mesh data/bunny.orig.m, compute a progressive mesh, then the spherical parameterization data/bunny.sphparam.m.'
bin/meshtopm.sh data/bunny.orig.m -minqem -vsgeom -dihallow | SphereParam - -rot data/bunny.s3d -split_meridian >data/bunny.sphparam.m


echo 'Resample the bunny spherical parameterization into a remesh and an associated normal map.'

SphereSample -domain octaflat -egrid 128 -sample_map data/octaflat_eg128.uv.sphparam.m -param data/bunny.sphparam.m -rot data/bunny.s3d -keys imageuv -remesh | Filtermesh -renamekey v imageuv uv >data/bunny.spheresample.remesh.m

SphereSample -domain octaflat -grid 1024 -domain_file data/octaflat_eg128.uv.sphparam.m -param data/bunny.sphparam.m -signal N -write_texture data/bunny.spheresample.octaflat.unrotated.normalmap.png


echo 'Create a longitude-latitude normal map to use on the original mesh.'

SphereSample -grid 1024 -param data/bunny.sphparam.m -signal N -write_lonlat_texture data/bunny.lonlat.unrotated.normalmap.png

echo 'Create a progressive mesh by minimizing an "Appearance-preserving simplification" (APS) metric.'

bin/meshtopm.sh data/bunny.sphparam.m -minaps -nominii1 -strict 2 >data/bunny.split_meridian.pm


echo '.'
echo 'Use view_spherical_param_bunny.sh to see the results.'
echo '.'
