<?php

/**
 * Implements hook_drush_command().
 */
function apachesolr_work_drush_command() {
  $items = array();
  $items['apachesolr-work-queue-content'] = array(
    'description' => '',
    'callback' => 'apachesolr_work_queue_content',
    'arguments' => array(),
    'options' => array(),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'aliases' => array(''),
  );
  return $items;
}

/**
 * This can be called:
 * drush -l http://spellbound.dbo apachesolr-work-queue-content
 */
function apachesolr_work_queue_content() {
  _apachesolr_work_queue_content();
}
