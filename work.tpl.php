<?php
  // work entity template
  $content = $element->content;
?>

<div>Title: <h2><?php echo render($content['title']); ?></div></h2>

<?php if (isset($content['alpha_title'])): ?>
  <div>Alpha title: <h3><?php echo render($content['alpha_title']); ?></h3></div>
<?php endif; ?>

<?php if (isset($content['production_date'])): ?>
  <div>Production date: <h3><?php echo render($content['production_date']); ?></h3></div>
<?php endif; ?>

<?php if (isset($content['alternative_titles'])): ?>
  <div>Alternative titles:</div><div><h3><?php echo render($content['alternative_titles']); ?></h3></div>
<?php endif; ?>

 
  
<!-- get genre terms -->
<?php if (isset($content['terms']['cid_genre'])): ?>
  <div><h3>Genres</h3></div>
  <div>
  <?php foreach($content['terms']['cid_genre'] as $term): ?>
    <div><?php echo $term['link']; ?></div>
  <?php endforeach; ?>
  </div>
<?php endif; ?>

  
<!-- get subject terms -->
<?php if (isset($content['terms']['subject'])): ?>
  <div><h3>Subjects</h3></div>
  <div>
  <?php foreach($content['terms']['subject'] as $term): ?>
    <div><?php echo $term['link']; ?></div>
  <?php endforeach; ?>
  </div>
<?php endif; ?>

  
  
<!-- related works and agents -->
<?php if (!empty($content['relations']['work'])): ?>
  <div><h3>Related Works</h3></div>
  <div>
  <?php foreach($content['relations']['work'] as $relation): ?>
    <?php echo $relation; ?><br />
  <?php endforeach; ?>
  </div>
<?php endif; ?>

<?php if (!empty($content['relations']['agent'])): ?>
  <div><h3>Related Agents</h3></div>
  <div>
  <?php foreach($content['relations']['agent'] as $relation): ?>
    <?php echo $relation; ?><br />
  <?php endforeach; ?>
  </div>
<?php endif; ?>


