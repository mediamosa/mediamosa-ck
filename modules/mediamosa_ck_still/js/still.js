
// JQuery extension URLEncode.
jQuery.extend({URLEncode:function(c){var o='';var x=0;c=c.toString();var r=/(^[a-zA-Z0-9_.]*)/;
while(x<c.length){var m=r.exec(c.substr(x));
  if(m!=null && m.length>1 && m[1]!=''){o+=m[1];x+=m[1].length;
  }else{if(c[x]==' ')o+='+';else{var d=c.charCodeAt(x);var h=d.toString(16);
  o+='%'+(h.length<2?'0':'')+h.toUpperCase();}x++;}}return o;},
URLDecode:function(s){var o=s;var binVal,t;var r=/(%[^%]{2})/;
while((m=r.exec(o))!=null && m.length>1 && m[1]!=''){b=parseInt(m[1].substr(1),16);
t=String.fromCharCode(b);o=o.replace(m[1],t);}return o;}
});

// escape does not escape like urlencode.
function media_mediamosa_urlencode(str) {
	return escape(str).replace(/\+/g,'%2B').replace(/%20/g, '+').replace(/\*/g, '%2A').replace(/\//g, '%2F').replace(/@/g, '%40');
}

jQuery(document).ready(function() {
	jQuery("#edit-domain-realm").bind("keydown", function(event) {
      // track enter key
      var keycode = (event.keyCode ? event.keyCode : (event.which ? event.which : event.charCode));
      if (keycode == 13) { // keycode for enter key
         // force the 'Enter Key' to implicitly click the Update button
         document.getElementById('edit-domain-realm-submit').click();
         return false;
      } 
      else  {
         return true;
      }
   }); // end of function

}); // end of document ready

function media_mediamosa_still_hideshow_item(name, show) {
	obj = document.getElementById(name);
	if (obj) {
		obj.style.display = show ? "block" : "none";
	}
}

function media_mediamosa_still_hide_by_type(obj) {
	
	switch (jQuery(obj).val()) {
		case 'NONE':
			media_mediamosa_still_hideshow_item('show-media_mediamosa-frametime', true); // None.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-per-mediafile', false); // Normal.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-every-second', false); // Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-start-time', false); // Normal / Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-end-time', false); // Normal / Second.
			break;

		case 'NORMAL':
			media_mediamosa_still_hideshow_item('show-media_mediamosa-frametime', false); // None.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-per-mediafile', true); // Normal.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-every-second', false); // Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-start-time', true); // Normal / Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-end-time', true); // Normal / Second.
			break;
	
		case 'SECOND':
			media_mediamosa_still_hideshow_item('show-media_mediamosa-frametime', false); // None.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-per-mediafile', false); // Normal.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-every-second', true); // Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-start-time', true); // Normal / Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-end-time', true); // Normal / Second.
			break;

		case 'SCENE':
			media_mediamosa_still_hideshow_item('show-media_mediamosa-frametime', false); // None.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-per-mediafile', false); // Normal.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-still-every-second', false); // Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-start-time', false); // Normal / Second.
			media_mediamosa_still_hideshow_item('show-media_mediamosa-end-time', false); // Normal / Second.
			break;
	}
}