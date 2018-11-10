
var x, lt;

x = null;

function loadMainInfo(p)
{
	var a = '';

	if (loadMainInfo.arguments.length == 1) {
		a = p;
		clearTimeout(lt);
	}
	if (x != null) {
		x.abort();
	}
	x = new XMLHttpRequest();
	x.onreadystatechange = function () {
		if (x.readyState == 4 && x.status == 200) {
		    document.getElementById('info').innerHTML = x.responseText;
		}
	};
	x.open('GET', 'MainInfo' + a, true);
	x.send();
	lt = setTimeout(loadMainInfo, 5000);
}

function loadSettingsInfo(p)
{
    if (x != null) {
        x.abort();
    }
    x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == 4 && x.status == 200) {
            document.getElementById('info').innerHTML = x.responseText;
        }
    };
    x.open('GET', 'SettingsInfo', true);
    x.send();
}

function loadInfoInfo(p)
{
    if (x != null) {
        x.abort();
    }
    clearTimeout(lt);
    x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == 4 && x.status == 200) {
            document.getElementById('info').innerHTML = x.responseText;
        }
    };
    x.open('GET', 'InfoInfo', true);
    x.send();
    lt = setTimeout(loadInfoInfo, 5000);
}

var sn = 0;
var id = 0;

function loadConsoleInfo(p)
{
	var c, o, t;

	clearTimeout(lt); o = '';
	t = document.getElementById('t1');
	if (p == 1) {
	    c = document.getElementById('c1');
		o = '&c1=' + encodeURIComponent(c.value);
		c.value = '';
		t.scrollTop = sn;
	}
	if (t.scrollTop >= sn) {
		if (x != null) {
			x.abort();
		}
		x = new XMLHttpRequest();
		x.onreadystatechange = function () {
			if (x.readyState == 4 && x.status == 200) {
				var z, d; d = x.responseXML;
				id = d.getElementsByTagName('i')[0].childNodes[0].nodeValue;
				if (d.getElementsByTagName('j')[0].childNodes[0].nodeValue == 0) {
					t.value = '';
				}
				z = d.getElementsByTagName('l')[0].childNodes;
				if (z.length > 0) {
					t.value += decodeURIComponent(z[0].nodeValue);
				}
				t.scrollTop = 99999;
				sn = t.scrollTop;
			}
		};
		x.open('GET', 'ConsoleInfo?c2=' + id + o, true); x.send();
	}
	lt = setTimeout(loadConsoleInfo, 5000);
	return false;
}

function loadRestartInfo()
{
    if (x != null) {
        x.abort();
    }
    x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == 4 && x.status == 200) {
            document.getElementById('info').innerHTML = x.responseText;
        }
    };
    x.open('GET', 'RestartInfo', true);
    x.send();
}
