<!DOCTYPE html>
<html>
<head>
<meta charset='utf-8'/>
<title>GPU accelerated stereo matching</title>
<script type="text/javascript">
	var e = <?php if(isset($_GET["e"])){ echo $_GET["e"]; }else{ echo "1"; }; ?>;
	var b = <?php if(isset($_GET["b"])){ echo $_GET["b"]; }else{ echo "1"; }; ?>;
	var p = <?php if(isset($_GET["p"])){ echo $_GET["p"]; }else{ echo "1"; }; ?>;
</script>
<script type="text/javascript" src="method<?php if(isset($_GET["m"])){ echo $_GET["m"]; }else{ echo "7"; }; ?>.js"></script>
<script type="text/javascript" src="lib.js" /></script>
</head>

<body style='margin: 0px' onload='main()'>
	<div style="width: 100%; text-align: center;">
		<canvas id='screen' style='background-color: black;'></canvas>
	</div>
</body>
</html>
