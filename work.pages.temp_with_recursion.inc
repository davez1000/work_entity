<?php

/**
 * Menu callback; Show list of work types we can add.
 */
function work_add_page() {
  $item = menu_get_item();
  $links = system_admin_menu_block($item);

  foreach ($links as $link) {
    $items[] = l($link['title'], $link['href'], $item['localized_options']) . ': ' . filter_xss_admin($link['description']);
  }

  return theme('item_list', array('items' => $items));
}

/**
 * Present an work submission form.
 */
function work_add($type) {
  global $user;

  $types = work_types();
  $type = isset($type) ? str_replace('-', '_', $type) : NULL;
  if (empty($types[$type])) {
    return MENU_NOT_FOUND;
  }

  $work = entity_get_controller('work')->create(array('type'=>$type));

  drupal_set_title(t('Create @name', array('@name' => $types[$type]->name)), PASS_THROUGH);
  return drupal_get_form($type . '_work_form', $work);
}

/**
 * Menu callback; presents the work editing form, or redirects to delete confirmation.
 *
 * @param $work
 *   The work object to edit.
 */
function work_page_edit($work) {
  $types = work_types();
  drupal_set_title(t('<em>Edit @type</em> @title', array('@type' => $types[$work->type]->name, '@title' => $work->title)), PASS_THROUGH);

  return drupal_get_form($work->type . '_work_form', $work);
}

/**
 * Form builder; Displays the work add/edit form.
 *
 * @param $form
 * @param $form_state
 * @param $work
 *   The work object to edit, which may be brand new.
 */
function work_form($form, &$form_state, $work) {

  // Set the id and identify this as an work edit form.
  $form['#id'] = 'work-form';

  // Save the work for later, in case we need it.
  $form['#work'] = $work;
  $form_state['work'] = $work;

  // Common fields.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Display title'),
    '#default_value' => $work->title,
    '#weight' => -5,
    '#required' => TRUE,
  );
  $form['identifying_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifying title'),
    '#default_value' => $work->identifying_title,
    '#weight' => -5,
    '#required' => TRUE,
    '#disabled' => TRUE,
  );
  $form['alpha_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Alphabetic title'),
    '#description' => t('The title used for sorting the works.'),
    '#default_value' => $work->alpha_title,
    '#weight' => -4,
    '#required' => FALSE,
    '#disabled' => TRUE,
  );
  if (isset($work->alternative_titles)) {
    if (! empty($work->alternative_titles)) {
      $form['alternative_titles'] = array(
        '#type' => 'fieldset',
        '#title' => t('Alternative titles'),
        '#weight' => -3,
      );
      
      foreach ($work->alternative_titles as $key => $alt_title) {
        $form['alternative_titles']['title' . $key] = array(
          '#type' => 'textfield',
          '#title' => t($alt_title->type),
          '#default_value' => $alt_title->title,
          '#disabled' => TRUE,
        );
      }
    }
  }
  $form['production_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Production date'),
    '#description' => t('The production date for the work.'),
    '#default_value' => $work->production_date,
    '#weight' => -2,
    '#required' => FALSE,
    '#disabled' => TRUE,
  );
  $form['revision'] = array(
    '#access' => user_access('administer works'),
    '#type' => 'checkbox',
    '#title' => t('Create new revision'),
    '#default_value' => 0,
  );

  // Add the buttons.
  $form['buttons'] = array();
  $form['buttons']['#weight'] = 100;
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('work_form_submit'),
  );
  if (!empty($work->id)) {
    $form['buttons']['delete'] = array(
      '#access' => user_access('delete works'),
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('work_form_delete_submit'),
    );
  }

  $form['#validate'][] = 'work_form_validate';

  field_attach_form('work', $work, $form, $form_state);

  return $form;
}

function work_form_validate($form, &$form_state) {
  $work = $form_state['work'];

  // Field validation.
  field_attach_form_validate('work', $work, $form, $form_state);
}


function work_form_submit($form, &$form_state) {
  global $user;

  $work = &$form_state['work'];

  // Set the work's uid if it's being created at this time.
  if (empty($work->uid)) {
    $work->uid = $user->uid;
  }

  $work->title = $form_state['values']['title'];
  if (!empty($form_state['values']['identifying_title'])){
    $work->identifying_title = $form_state['values']['identifying_title'];
  }
  if (!empty($form_state['values']['alpha_title'])){
    $work->alpha_title = $form_state['values']['alpha_title'];
  }
  
  if (!empty($form_state['values']['production_date'])) {
    $work->production_date = $form_state['values']['production_date'];
  }

  $work->revision = $form_state['values']['revision'];

  // Notify field widgets.
  field_attach_submit('work', $work, $form, $form_state);

  // Save the work.
  work_save($work);

  // Notify the user.
  drupal_set_message(t('work saved.'));

  $form_state['redirect'] = 'work/' . $work->id;
}


function work_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $work = $form['#work'];
  $form_state['redirect'] = array('work/' . $work->id . '/delete', array('query' => $destination));
}

/**
 * Displays an work.
 *
 * @param $work
 *   The work object to display.
 * @param $view_mode
 *   The view mode we want to display.
 */
function work_page_view($work, $view_mode = 'full') {
  // Remove previously built content, if exists.
  $work->content = array();
  
  if ($view_mode == 'teaser') {
//    $work->content['title']['#markup'] = l(filter_xss($work->title), 'work/' . $work->id);
    $work->content['title'] = array(
      '#markup' => l(filter_xss($work->title), 'work/' . $work->id) . '<br />',
      '#weight' => -5,
    );
  }
  else {
    // full view mode

    $work->content['title']['#markup'] = $work->title;
    
    $work->content['alpha_title']['#markup'] = $work->alpha_title;

    if (isset($work->alternative_titles) && !empty($work->alternative_titles)) {
      $alt_t = array();
      foreach($work->alternative_titles as $alt_title) {
        $alt_t[] = $alt_title->title;
      }
      $work->content['alternative_titles'] = implode($alt_t, '<br />');
    }
    
    
    if (isset($work->production_date) && !empty($work->production_date)) {
      if (preg_match('/^(\d{4})\-(\d{2})\-(\d{2})$/', $work->production_date, $matches)) {
        $prod_date = date('j F Y', mktime(0, 0, 0, $matches[2], $matches[3], $matches[1]));
        
      }
      else if (preg_match('/^(\d{4})$/', $work->production_date, $matches)) {
        $prod_date = $matches[1];
      }
      $work->content['production_date']['#markup'] = $prod_date;
    }
    

    // get terms to pass to the template
//    if (isset($work->terms) && !empty($work->terms)) {
//      $work->content['terms'] = $work->terms;
//    }
    
    
    // additional field api fields
    if (!empty($work->alternate_title)) {
      $work->content['alternate_title'] = $work->alternate_title['und'][0]['value'];
    }
    
    if (!empty($work->introduction)) {
      if (!empty($work->introduction['und'][0]['summary'])) {
        $work->content['introduction_summary'] = $work->introduction['und'][0]['summary'];
      }
      if (!empty($work->introduction['und'][0]['value'])) {
        $work->content['introduction'] = $work->introduction['und'][0]['value'];
      }
    }
    
    if (!empty($work->synopsis)) {
      $work->content['synopsis'] = $work->synopsis['und'][0]['value'];
    }
    
    if (!empty($work->external_link)) {
      $links = array();
      foreach($work->external_link['und'] as $link) {
        $links[] = l($link['title'], $link['url']);
      }
      $work->content['external_links'] = $links;
    }
    
    if (!empty($work->one_liner)) {
      $work->content['one_liner'] = $work->one_liner['und'][0]['value'];
    }
    
    if (isset($work->primary_image['und'][0]['uri'])) {
      $work->content['primary_image'] = $work->primary_image['und'][0]['uri'];
    }
    
    if (!empty($work->year_term)) {
      $term = taxonomy_term_load($work->year_term['und'][0]['tid']);
      $work->content['year'] = l($term->name, 'taxonomy/term/' . $term->tid);
    }
    
    if (!empty($work->work_category)) {
      $term = taxonomy_term_load($work->work_category['und'][0]['tid']);
      $work->content['work_category'] = l($term->name, 'taxonomy/term/' . $term->tid);
    }
    
    
    // terms grouped by type
    if (isset($work->taxonomy_tids) && !empty($work->taxonomy_tids)) {
      $terms = array();
      $tid_count = 0;
//      $build_terms = '<ul>';
      $i = 0;
      foreach($work->taxonomy_tids as $tid) {
        $terms_with_parents = array_reverse(taxonomy_get_parents_all($tid));

        $terms_count = count($terms_with_parents);
        if ($terms_count > 1) {
//          $build_terms .= _terms_recurse($terms_with_parents, $terms_count);
          $terms[$terms_with_parents[0]->vocabulary_machine_name][$i] = _terms_recurse($terms_with_parents, $terms_count);
        }
        else {
//          $build_terms .= '<li>' . l($terms_with_parents[0]->name, 'taxonomy/term/' . $terms_with_parents[0]->tid) . '</li>' . PHP_EOL;
          $terms[$terms_with_parents[0]->vocabulary_machine_name][$i] = '<li>' . l($terms_with_parents[0]->name, 'taxonomy/term/' . $terms_with_parents[0]->tid) . '</li>' . PHP_EOL;
        }
        $tid_count++;
        $i++;
      }
//      print_r($terms); exit;

//      $build_terms .= '</ul>';
//      echo $build_terms; exit;

      $work->content['terms'] = $terms;
    }


    
    // relations
//    $relations = module_invoke('relation_helper', 'get_endpoints', $work->id, 'work');
//
//    if (!empty($relations)) {
//      $i = 0;
//      foreach($relations as $relation) {
//        foreach($relation['entity'] as $relation_entity) {
//          $work->content['relations'][$relation['entity_type']][$i] = l($relation_entity->title, $relation['entity_type'] . '/' . $relation_entity->id);
//        }
//        $i++;
//      }
//    }
  }
  
  
  // Build fields content.
  field_attach_prepare_view('work', array($work->id => $work), $view_mode);
  entity_prepare_view('work', array($work->id => $work));
  $work->content += field_attach_view('work', $work, $view_mode);
  
  
  /*
   * Specify the theme to use and set the #element. Note that the key 
   * you use to pass the entity object must match the key you set in the 
   * variables in entity_theme(). So in the case below, we use the key 
   * named #element because in entity_theme() we set the following code:
   *
   * array(
   *   'entity' => array(
   *     'variables' => array('element' => null),
   *     'template' => 'entity'
   *   ),
   * );
   */
  $work->content += array(
    '#theme'     => 'work',
    '#element'   => $work,
    '#view_mode' => 'full',
    '#language'  => NULL,
  );
  
  return $work->content;
}



function _terms_recurse($terms, $terms_count, $key = 0) {
  if ($key > 0) {
    $build_terms = '<ul>' . PHP_EOL;
  }
  $build_terms .= '<li>' . l($terms[$key]->name, 'taxonomy/term/' . $terms[$key]->tid);
  if (isset($terms[$key++])) {
    $build_terms .= _terms_recurse($terms, $terms_count, $key++);
  }
  else {
    return;
  }
  $build_terms .= '</li>' . PHP_EOL;
  if ($key > 0 && $key <= $terms_count) {
    $build_terms .= '</ul>' . PHP_EOL;
  }
  return $build_terms;
}


/**
 * Form bulder; Asks for confirmation of work deletion.
 */
function work_delete_confirm($form, &$form_state, $work) {
  $form['#work'] = $work;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['id'] = array('#type' => 'value', '#value' => $work->id);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $work->title)),
    'work/' . $work->id,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Executes work deletion.
 */
function work_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $work = work_load($form_state['values']['id']);
    work_delete($form_state['values']['id']);
    watchdog('work', '@type: deleted %title.', array('@type' => $work->type, '%title' => $work->title));

    $types = work_types();
    drupal_set_message(t('@type %title has been deleted.', array('@type' => $types[$work->type]->name, '%title' => $work->title)));
  }

  $form_state['redirect'] = '<front>';
}

/**
 * Menu callback; Displays a listing of recent works.
 *
 * This doesn't really work yet because our presentation code doesn't show
 * the title.
 */
function work_page_list_recent() {
  $content = array();

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'work')
    ->propertyOrderBy('created', 'DESC')
    ->range(0, 20);
  $result = $query->execute();

  $works = work_load_multiple(array_keys($result['work']));
  foreach ($works as $work) {
    $content[$work->id] = work_page_view($work, 'teaser');
  }

  return $content;
}

/**
 * Menu callback; Displays a list of works.
 */
function work_page_list() {
  
  global $base_url;

  $rows = array();

  $header = array(
    t('Title'),
    t('Type'),
    t('Edited'),
  );

  $query = db_select('work', 'w')
    ->fields('w', array('id', 'type', 'title', 'changed'))
    ->condition('enhanced', 1, '=')
    ->execute();
  
  foreach ($query as $work) {

    $cells = array();
    $cells[] = l($work->title, $base_url . '/work/' . $work->id);
    $cells[] = $work->type;
    $cells[] = format_date($work->changed, 'short');
    $rows[] = array('data' => $cells);
  }
  
  $build['work_list_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('id' => ''),
    '#empty' => t('No works available.'),
  );
  return $build;
}
