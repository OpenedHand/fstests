#/bin/sh

echo "framebuffer via VT switch from X, Single Blit, na"

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-fb --verbose
done

for ORIENTATION in normal left right inverted
do
  echo
  echo "X MIT-SHM PutImage, Single Blit, $ORIENTATION"
  echo
  xrandr -o $ORIENTATION
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose
  done
  echo
  echo "X MIT-SHM PutImage, Multiple Blit (x), $ORIENTATION"
  echo
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose --multiblit=x
  done
  echo "X MIT-SHM PutImage, Multiple Blit (y), $ORIENTATION"
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose --multiblit=y
  done

done

for ORIENTATION in normal left right inverted
do
  echo
  echo "X PutImage, Single Blit, $ORIENTATION"
  echo
  xrandr -o $ORIENTATION
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose --no-shm
  done
  echo
  echo "X PutImage, Multiple Blit(x), $ORIENTATION"
  echo
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose --no-shm --multiblit=x
  done
  echo
  echo "X PutImage, Multiple Blit(y), $ORIENTATION"
  echo
  for i in 1 2 3 4 5 6 7 8 9 10
  do
  ./test-x --verbose --no-shm --multiblit=y
  done
done

xrandr -o normal

if [ $DO_SHM_TESTS = 'y' ] 
then

echo
echo "X + SDL, Single Blit, normal "
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
  ./test-sdl --verbose -w 240 -h 320
done

echo
echo "X + SDL, Multiple Blit(x), normal "
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
  ./test-sdl --verbose --multiblit=x -w 240 -h 320
done

echo
echo "X + SDL, Multiple Blit(y), normal "
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
  ./test-sdl --verbose --multiblit=y -w 240 -h 320
done

fi

killall gpe-dm
killall Xfbdev

echo "framebuffer no X, Single Blit, na"

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-fb --verbose
done

if [ $DO_SHM_TESTS = 'y' ] 
then

export SDL_NOMOUSE=1

echo "framebuffer + SDL (swsurface), Single Blit, na"

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-sdl -w 320 -h 240
done

echo
echo "framebuffer + SDL (hwsurface), Single Blit, na"
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-sdl --verbose -w 320 -h 240 --hwsurface
done

echo
echo "framebuffer + SDL (hwsurface), Multiple Blit(x), na"
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-sdl --verbose -w 320 -h 240 --multiblit=x --hwsurface
done

echo
echo "framebuffer + SDL (hwsurface), Multiple Blit(y), na"
echo

for i in 1 2 3 4 5 6 7 8 9 10
do
./test-sdl --verbose -w 320 -h 240 --multiblit=y --hwsurface
done

fi



