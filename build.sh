mkdir -p build
pushd build
echo "Building library"
g++ -c ../src/untar.cpp -I../include -o untar.o

echo "Default demo"
echo "... Building"
g++ -c ../example/main.cpp -I../include -o demo.o
echo "... Linking"
g++ demo.o untar.o -o demo

echo "Wrapped_find example"
echo "... Building"
g++ -c ../example/wrapped_find.cpp -I../include -o wrapped_find.o
echo "... Linking"
g++ wrapped_find.o untar.o -o wrapped_find

echo "Prepare test data"
cp ../example/example.tar .

echo "You should be able to test the library with: ./build/demo ./build/example.tar"
popd
