if [[ ! -d build_test ]]; then
  mkdir build_test
fi

cd build_test
cmake -DBUILD_TEST=ON ..
make -j4
stat=$?
cd -

if [[ $stat -ne 0 ]]; then
  echo "Compile failed."
  exit 1
fi

./build_test/test/unittest/gmif_unittest