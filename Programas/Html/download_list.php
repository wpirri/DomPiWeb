<?php
$TITLE='Descargas'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='download_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='download_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='download_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
    function OnLoad() {
        <?php
            $output = "<div class=abm-table-title>&nbsp;Downloads</div>\\n";
            $output = $output."<table class=abm-list-table>\\n";
            $output = $output."<tr><th>Archivo</th><th>Bajar</th></tr>\\n";

            $dir = scandir($DOWNLOAD_FOLDER);
            if($dir != false)
            {
                foreach($dir as $value)
                {
                    if(is_file($DOWNLOAD_FOLDER."/".$value))
                    {
                        $output = $output."<tr><td>".$value."</td>";
                        $output = $output."<td><a target=_blank href=\"".$DOWNLOAD_FOLDER."/".$value."\"><img src=\"images/download.png\"></a></td></tr>\\n";
                    }

                }

            }
            $output = $output."</table>\\n";
        ?>
        document.getElementById('download_list_table_div').innerHTML = '<?php print($output); ?>';
    }
</script>

</body>

<?php
include('foot.php');
?>
