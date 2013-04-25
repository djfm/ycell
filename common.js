function sum(array)
{
    var s = 0;
    for(var i in array)
    {
        s += array[i];
    }
    return s;
}

function average(array)
{
    return sum(array)/array.length;
}
