function max(num1, num2)
  if(num1 >= num2) then
    return num1
  else
    return num2
  end
end

function min(num1, num2)
  if(num1 <= num2) then
    return num1
  else
    return num2
  end
end


tuples = {}
for t1 = 3, 9, 3 do
  for t2 = 2, 10, 2 do
    tuples[#tuples+1] = {t1,t2};
  end
end
--<LINE NOT MATCHED>
--<LINE NOT MATCHED>
