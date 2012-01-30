<?php
  // work entity template
  $content = $element->content;
?>

<div>title:<?php echo render($content['title']); ?></div>

<?php if (isset($content['production_date'])): ?>
  <div>production date:<?php echo render($content['production_date']); ?></div>
<?php endif; ?>

<?php if (isset($content['identifying_title'])): ?>
  <div>identifying_title: <?php echo render($content['identifying_title']); ?></div>
<?php endif; ?>
  
<?php if (isset($content['alternate_title'])): ?>
  <div>alternate_title: <?php echo render($content['alternate_title']); ?></div>
<?php endif; ?>
  
<?php if (isset($content['introduction_summary'])): ?>
  <div>introduction_summary: <?php echo render($content['introduction_summary']); ?></div>
<?php endif; ?>
<?php if (isset($content['introduction'])): ?>
  <div>introduction: <?php echo render($content['introduction']); ?></div>
<?php endif; ?>

<?php if (isset($content['synopsis'])): ?>
  <div>synopsis: <?php echo render($content['synopsis']); ?></div>
<?php endif; ?>

<?php if (isset($content['external_links'])): ?>
  <div>External links:</div>
  <?php foreach($content['external_links'] as $link): ?>
    <div><?php echo $link; ?></div>
  <?php endforeach; ?>
<?php endif; ?>

<?php if (isset($content['one_liner'])): ?>
  <div>one_liner: <?php echo render($content['one_liner']); ?></div>
<?php endif; ?>
  
<?php if (isset($content['primary_image'])): ?>
  <div>image: <?php echo theme('image_style', array('style_name' => 'sys_common_gallery_image', 'path' => render($content['primary_image']))); ?></div>
  <div>image (URL only): <?php echo image_style_url('sys_common_gallery_image', render($content['primary_image'])); ?></div>
<?php endif; ?>
  
<?php if (isset($content['year'])): ?>
  <div>year: <?php echo render($content['year']); ?></div>
<?php endif; ?>

<?php if (isset($content['work_category'])): ?>
  <div>work_category: <?php echo render($content['work_category']); ?></div>
<?php endif; ?>  


<?php // if (isset($content['alpha_title'])): ?>
  <?php // echo render($content['alpha_title']); ?>
<?php // endif; ?>



<?php // if (isset($content['alternative_titles'])): ?>
  <?php // echo render($content['alternative_titles']); ?>
<?php // endif; ?>

 
<!-- get genre terms -->
<h3>Genres</h3>
<?php if (isset($content['terms']['cid_genre'])): ?>
  <?php foreach($content['terms']['cid_genre'] as $term): ?>
    <?php echo $term; ?><br />
  <?php endforeach; ?>
 
<?php endif; ?>

  
<!-- get subject terms -->
<h3>Subjects</h3>
<?php if (isset($content['terms']['subject'])): ?>
  <?php foreach($content['terms']['subject'] as $term): ?>
    <?php echo $term; ?><br />
  <?php endforeach; ?>
<?php endif; ?>

  
  
<!-- related works -->
<h3>Related Works</h3>
<?php if (!empty($content['relations']['work'])): ?>
  
  <div>
  <?php foreach($content['relations']['work'] as $relation): ?>
    <?php echo $relation; ?><br />
  <?php endforeach; ?>
  </div>
<?php endif; ?>


<!-- related agents -->
<h3>Related Agents</h3>
<?php if (!empty($content['relations']['agent'])): ?>

  <div>
  <?php foreach($content['relations']['agent'] as $relation): ?>
    <?php echo $relation; ?><br />
  <?php endforeach; ?>
  </div>
<?php endif; ?>


