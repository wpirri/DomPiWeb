<?php
$TITLE='Estado'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='status_list_back_btn' class='back-btn' onclick="window.location.replace('config.php');" >
	<img id='status_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='status_list_add_btn' class='abm-add-btn' onclick="window.location.replace('status_add.php');" >
	<img id='status_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='status_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function RefreshData(msg) {
      respJSON = JSON.parse(msg).response;
      for (var i = 0; i < respJSON.length; i++) {
        if(respJSON[i].Id > 0)
        {
          document.getElementById(respJSON[i].Objeto).src = '/images/' + respJSON[i].FileNamePrefix + respJSON[i].Estado + '.' + respJSON[i].FileNameExt;
        }
      }
    }

    function LoadData(msg) {
      respJSON = JSON.parse(msg).response;
      var table = '<table>\n';
      table += '<tr><td>Objeto</td><td>Estado</td></tr>\n';
      for (var i = 0; i < respJSON.length; i++) {
        if(respJSON[i].Id > 0)
        {
          table += '<tr>';  
          table += '<td>' + respJSON[i].Objeto + '</td>';  
          table += '<td><img id=\"' + respJSON[i].Objeto + '\" src=\"images/' + respJSON[i].FileNamePrefix + respJSON[i].Estado + '.' + respJSON[i].FileNameExt  + '\"></td>';  
          table += '</tr>\n';
        }
      }
      table += '</table>';  
      
      document.getElementById('status_list_table_div').innerHTML = table;
      newAJAXCommand('/cgi-bin/statusio.cgi', RefreshData, true);
    }

    function OnLoad() {
      newAJAXCommand('/cgi-bin/statusio.cgi', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
