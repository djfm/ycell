function sum(array)
{
    var s = 0;
    for(var i in array)
    {
        if(typeof(array[i]) == "object")
        {
            s += sum(array[i]);
        }
        else if(typeof(array[i]) == "number")
        {
            s += array[i];
        }
    }
    return s;
}

function count(array)
{
    var s = 0;
    for(var i in array)
    {
        if(typeof(array[i]) == "object")
        {
            s += count(array[i]);
        }
        else if(typeof(array[i]) == "number")
        {
            s += 1;
        }
    }
    return s;
}

function average(array)
{
    return sum(array)/count(array);
}

Array.prototype.default_map = Array.prototype.map;

Array.prototype.map = function(arg)
{
    if(typeof(arg) == "function")
    {
        return this.default_map(arg);
    }
    else if(typeof(arg) == "string")
    {
        return this.default_map(function(row){
            var code = arg.replace(/\b[A-Z]+\b/g,function(match){
                var val;
                if(typeof(row) == 'object')
                {
                    var i = match.charCodeAt(0) - 'A'.charCodeAt(0);
                    val = row[i];
                }
                else val = row;

                if(isNaN(Number(val)))
                {
                    val = JSON.stringify(val);
                }

                return val;
            });
            return eval(code);
        });


    }
    else
    {
        return "#ERR";
    }
}
