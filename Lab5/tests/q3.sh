test()
{
    # Packet count after transmission through both the token bucket and fifo queue
    PACKET_COUNT_QUEUE=$(./shape 500 $1 < arrivals.txt | ./fifo 1000 10.0 | wc -l)
    # Pakcet count originally
    PACKET_COUNT_ORIGINAL=$(wc -l < arrivals.txt)

    # Comparing if both the packet counts are equal or not
    if [ $PACKET_COUNT_ORIGINAL -eq $PACKET_COUNT_QUEUE ]; then echo 1
    else echo 0; fi

}

# Lower index for space - smallest 6 digit decimal
lo=0.000001
# Higher index for search space - highest digit.
hi=10
# Random mid ini
mid=2
while [ $(echo "$hi > $lo" | bc -l) ]; do

  # if mid converges then break 
  prev=$(echo "scale=6; ($lo + $hi) / 2" | bc)   
  if [ $(echo "$mid == $prev" | bc) -eq 1 ]; then break; fi 
  mid=$(echo "scale=6; ($lo + $hi) / 2" | bc)

  # Store the result of the test function
  RESULT=`test $mid`

  # If result is 1 then move towards the lower end otherwise move towards higher end.
  if [ $RESULT -eq 1 ]; then lo=$mid; else hi=$mid; fi

done
echo $mid