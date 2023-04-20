sudo insmod calc.ko
sudo chmod 777 /dev/var2
sudo chmod 777 /proc/var2
RANDOM_NUMBER1=$((RANDOM%10))
RANDOM_NUMBER2=$((RANDOM%10))

EXPECTATION=$((RANDOM_NUMBER1+1))


echo "$RANDOM_NUMBER1+$RANDOM_NUMBER2" > /dev/var2


ACTUAL=$(cat /proc/var2)
if [[ "$EXPECTATION" = "$ACTUAL" ]]; then
    echo "Test is OK"
else
    echo "Test is failed" >&2
fi

