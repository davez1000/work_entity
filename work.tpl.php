<?php
  // work entity template
  $content = $element->content;
?>

<?php if ($view_mode == 'teaser'): // display 'teaser' view mode ?>

  <?php //$content = $variables['content']; ?>
  
  
  <div><?php echo render($content['title']); ?></div>
  <div><?php echo render($content['primary_image']); ?></div>
  <div><?php echo render($content['introduction_summary']); ?></div>



<?php else: // display 'full' view mode ?>


  
<!-- ############################################# TEMPLATE HEADER ############################################# -->  
  
  <div>title:<?php echo render($content['title']); ?></div>


  <?php if (isset($content['identifying_title'])): ?>
    <div>identifying_title: <?php echo render($content['identifying_title']); ?></div>
  <?php endif; ?>
    
    
  <?php if (isset($content['alpha_title'])): ?>
    <?php echo render($content['alpha_title']); ?>
  <?php endif; ?> 
    
    
  <?php if (isset($content['alternative_titles'])): ?>
    <?php echo render($content['alternative_titles']); ?>
  <?php endif; ?>
    
    
  <?php if (isset($content['production_date'])): ?>
    <div>production date:<?php echo render($content['production_date']); ?></div>
  <?php endif; ?>
    
  <!-- year -->
  
  <?php if (isset($content['year'])): ?>
    <div>year: <?php echo render($content['year']); ?></div>
  <?php endif; ?>
    
  <!-- work category -->
  
  <?php if (isset($content['work_category'])): ?>
    <div>work_category: <?php echo render($content['work_category']); ?></div>
  <?php endif; ?>
    
  <!-- Alternate Title -->

  <?php if (isset($content['alternate_title'])): ?>
    <div>alternate_title: <?php echo render($content['alternate_title']); ?></div>
  <?php endif; ?>
    

<!-- ################################################# CONTENT ################################################# -->
   

  <!-- Primary Image -->   
  
  <?php if (isset($content['primary_image'])): ?>
    <div>image: <?php echo theme('image_style', array('style_name' => 'dbo_common_gallery_image', 'path' => render($content['primary_image']))); ?></div>
    <div>image (URL only): <?php echo image_style_url('dbo_common_gallery_image', render($content['primary_image'])); ?></div>
  <?php endif; ?>    
    
  <!-- Intro -->   

  <?php if (isset($content['introduction_summary'])): ?>
    <div>introduction_summary: <?php echo render($content['introduction_summary']); ?></div>
  <?php endif; ?>
    
  <?php if (isset($content['introduction'])): ?>
    <div>introduction: <?php echo render($content['introduction']); ?></div>
  <?php endif; ?>
    
  <!-- Synopsis -->     

  <?php if (isset($content['synopsis'])): ?>
    <div>synopsis: <?php echo render($content['synopsis']); ?></div>
  <?php endif; ?>
    
  <!-- External Links -->     

  <?php if (isset($content['external_links'])): ?>
    <div>External links:</div>
    <?php foreach($content['external_links'] as $link): ?>
      <div><?php echo $link; ?></div>
    <?php endforeach; ?>
  <?php endif; ?>

  <!-- One Liner --> 
      
  <?php if (isset($content['one_liner'])): ?>
    <div>one_liner: <?php echo render($content['one_liner']); ?></div>
  <?php endif; ?>

  <!-- get genre terms -->
  
  <?php if (isset($content['terms']['cid_genre'])): ?>
    <ul>
      <?php foreach($content['terms']['cid_genre'] as $term): ?>
        <?php echo $term; ?>
      <?php endforeach; ?>
    </ul>
  <?php endif; ?>

  <!-- get subject terms -->

  <?php if (isset($content['terms']['subject'])): ?>
    <ul>
      <?php foreach($content['terms']['subject'] as $term): ?>
        <?php echo $term; ?>
      <?php endforeach; ?>
    </ul>
  <?php endif; ?>


  <!-- related works -->

  <?php if (!empty($content['relations']['work'])): ?>

    <div>
    <?php foreach($content['relations']['work'] as $relation): ?>
      <?php echo $relation; ?><br />
    <?php endforeach; ?>
    </div>
  <?php endif; ?>


  <!-- related agents -->

  <?php if (!empty($content['relations']['agent'])): ?>

    <div>
    <?php foreach($content['relations']['agent'] as $relation): ?>
      <?php echo $relation; ?><br />
    <?php endforeach; ?>
    </div>
  <?php endif; ?>


  
<?php endif; ?>
