echo "1" > /proc/calc/arg1
echo "3" > /proc/calc/arg2
echo "+" > /proc/calc/operation

cat /proc/calc/result
