<!DOCTYPE html>
<html>
<head>
<meta charset='utf-8'/>
<title>GPU accelerated stereo matching</title>
<script type="text/javascript">
	var edge = <?php if(isset($_GET["e"])){ echo $_GET["e"]; }else{ echo "1"; }; ?>;
	var blur = <?php if(isset($_GET["b"])){ echo $_GET["b"]; }else{ echo "1"; }; ?>;
	var resize = <?php if(isset($_GET["r"])){ echo $_GET["r"]; }else{ echo "0"; }; ?>;

	var pyramid = <?php if(isset($_GET["p"])){ echo $_GET["p"]; }else{ echo "4"; }; ?>;
	var d_0 = <?php if(isset($_GET["d0"])){ echo $_GET["d0"]; }else{ echo "0"; }; ?>;
	var d_times = <?php if(isset($_GET["dt"])){ echo $_GET["dt"]; }else{ echo "2"; }; ?>;

</script>
<script type="text/javascript" src="hierarchical.js"></script>
<script type="text/javascript" src="lib.js" /></script>
</head>

<body style='margin: 0px' onload='main()'>
	<div style="width: 100%; text-align: center;">
		<canvas id='screen' style='background-color: black;'></canvas>
	</div>
</body>
</html>
