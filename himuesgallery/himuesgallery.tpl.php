<?php
/**
 * @file
 */

//variables:
// $node
// $picturelist
// $path
$prefix="";
if (module_exists("colorbox")) {
	drupal_add_js("jQuery(document).ready(function(){
			jQuery(\"a[rel='cb_lightshow[himuesGallery]']\").colorbox({ slideshow:true, transition:'fade', speed:1000, slideshowSpeed:5000}); 
			jQuery(\"a[rel='cb_lightbox[himuesGallery]']\").colorbox({ transition:'fade', speed:1000, slideshowSpeed:5000 }); 
			jQuery(\"a[rel='cb_lightbox[]']\").colorbox({ rel: 'nofollow' }); 
			jQuery(\"a[rel='cb_youtube[]']\").colorbox({iframe:true, innerWidth:480, innerHeight:390}); 
	});"
	, 'inline');
  
	drupal_add_js("function youtubeFeedCallback(json){
    document.write('<a href=\"https://www.youtube.com/embed/' + json[\"data\"][\"id\"] + '?autoplay=1\" title=\"' + json[\"data\"][\"title\"] + '\" rel=\"cb_youtube[]\" target=\"_blank\" class=\"cboxElement\">');
    document.write('<img class=\"himuesgallery_picture_image\" src=\"' + json[\"data\"][\"thumbnail\"][\"hqDefault\"] + '\"></a>');
    document.write('<div class=\"himuesgallery_picture_description\">' + json[\"data\"][\"title\"] + '</div>');
  }", 'inline');

	$prefix="cb_";
}
//Workaround for Windows that uses "\" as path-separator
$path = str_replace('\\', '/', $path);
?>
<div class="himuesgallery_outer">

<div class="himuesgallery_text"><?php /*print $node->body;*/ isset($node->body) ? $node->body : NULL;?></div>
<?php
  foreach ($picturelist as $picture) {
?>
  <div class='himuesgallery_picture'>
    <?php
      if ($picture['type'] == 'youtube') {
    ?>
      <div class="himuesgallery_youtube_icon"></div>
      <script type="text/javascript" src="https://gdata.youtube.com/feeds/api/videos/<?php echo $picture['description']; ?>?v=2&alt=jsonc&callback=youtubeFeedCallback"></script>
    <?php
      } else {
    ?>
    <div class='himuesgallery_picture_image'>
      <a href="<?php $pic = drupal_encode_path($path . $node->gallery_path . "/" . $picture['File']); $pic = str_replace('%252F', '', $pic); echo $pic; ?>"
        <?php
        switch ($node->showtype) {
          case 0:
            echo "rel='" . $prefix . "lightbox[]'";
            break;

          case 1:
            echo "rel='" . $prefix . "lightbox[himuesGallery]'";
            break;

          default:
            echo "rel='" . $prefix . "lightshow[himuesGallery]'";
        }

        echo "  title='{$picture['description']}' name='{$picture['description']}'";
        ?>
 target="_blank">
          <img src="<?php  $thumb = drupal_encode_path($path . $node->gallery_path . "/thumbs/thumb." . $picture['File']);	$thumb = str_replace('%252F', '', $thumb); echo $thumb; ?>">
      </a>
    </div>
    <div class='himuesgallery_picture_description'><?php print $picture['description']; ?></div>
    <?php
      }
    ?>
  </div>
<?php
  } //end of foreeach
?>
</div>
<div class="content clearfix">
</div>