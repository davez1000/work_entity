<?php
// $Id$

/**
 * @file
 * Functions associated with the work services resource.
 */

/**
 * Access callback for the work resource.
 *
 * @param string $op
 *  The operation that's going to be performed.
 * @param array $args
 *  The arguments that will be passed to the callback.
 * @return bool
 *  Whether access is given or not.
 */
function _work_access($op, $args) {
  global $user;
  $access = FALSE;

  switch ($op) {
    case 'view':
      $access = user_access('view api works');
      break;
  }

  return $access;
}

/**
 * Callback for retrieving work resources.
 *
 * @param $web_id
 *   The web id of the requested work.
 * @return object
 */
function _work_retrieve($web_id) {

  /**
   * Construct the path to the current service endpoint so we can
   * include a full uri for referenced works/agents.
   */
  global $base_url;
  $url_parts = explode('/', current_path());
  $endpoint_path = $url_parts[0];
  $api_path = "";
  $endpoints = services_endpoint_load_all();
  foreach ($endpoints as $endpoint) {
    if ($endpoint->path == $endpoint_path) {
      $api_path = $base_url . '/' . $endpoint_path . '/';
      break;
    }
  }

  $work = NULL;
  $ids = idents_get_ids(array('type' => 'web_id', 'value' => $web_id));
  if (isset($ids->entity_id) && isset($ids->module)) {
    if (($ids->module == 'work') && (! empty($ids->entity_id))) {
      $work = work_load($ids->entity_id);
    }
  }

  // convert agent entity ids to web ids in the cast/credits
  if (isset($work->credits)) {
    // cast / credit
    foreach ($work->credits as $credit_type) {
      foreach ($credit_type as $credit_group) {
        foreach ($credit_group as $credit) {
          $ids = idents_get_ids_for_entity_by_module($credit->agent_id, 'agent');
          $credit->id = $ids->web_id;
          $credit->name = $credit->agent_name;
          $credit->uri = $api_path . 'agent/' . $ids->web_id;
          unset($credit->type);
          unset($credit->agent_id);
          unset($credit->credit_group);
          unset($credit->agent_name);
        }
      }
    }
  }

  // convert the primary image to a public url
  if (isset($work->primary_image['und'][0]['uri'])) {
    $work->primary_image = image_style_url('large', $work->primary_image['und'][0]['uri']);
  }
  else {
    unset($work->primary_image);
  }

  // tidy the term properties
  if (isset($work->taxonomy_terms)) {
    foreach ($work->taxonomy_terms as $vocab) {
      foreach ($vocab as $term) {
        $term->uri = $api_path . 'term/' . $term->tid;
        $term->vocabulary = $term->vocabulary_machine_name;
        unset($term->tid);
        unset($term->vid);
        unset($term->description);
        unset($term->format);
        unset($term->weight);
        unset($term->vocabulary_machine_name);
        unset($term->term_summary);
        unset($term->term_pullquote);
        unset($term->term_header_image);
        unset($term->work_ref);
        unset($term->agent_ref);
        unset($term->primary_image);
      }
    }
    $work->terms = $work->taxonomy_terms;
  }
  unset($work->taxonomy_terms);
  unset($work->taxonomy_tids);

  // move the field values out of ['und'][0] etc
  if (isset($work->introduction['und'][0]['value'])) {
    $work->introduction = $work->introduction['und'][0]['value'];
  }
  else {
    unset($work->introduction);
  }

  if (isset($work->synopsis['und'][0]['value'])) {
    $work->synopsis = $work->synopsis['und'][0]['value'];
  }
  else {
    unset($work->synopsis);
  }

  if (isset($work->external_link['und'][0]['value'])) {
    $work->external_link = $work->external_link['und'][0]['value'];
  }
  else {
    unset($work->external_link);
  }

  if (isset($work->one_liner['und'][0]['value'])) {
    $work->one_liner = $work->one_liner['und'][0]['value'];
  }
  else {
    unset($work->one_liner);
  }

  // Add related reviews and articles to the work object
  $related = dbo_relation_get_related_entities($work->id, 'work');
  $related_reviews = array();
  $related_articles = array();
  foreach($related as $id => $entity) {
    if ($entity['entity_type'] == 'node') {
      $node = node_load($entity['entity_id']);
      $r = new StdClass;
      $r->title = $node->title;
      if ($node->type == 'review') {
        $r->uri = $api_path . 'review/' . $node->idents['web_id'];
        $related_reviews[] = $r;
      }
      if ($node->type == 'article') {
        $r->uri = $api_path . 'article/' . $node->idents['web_id'];
        $related_articles[] = $r;
      }
    }
  }
  if (! empty($related_articles)) {
    $work->related_articles = $related_articles;
  }
  if (! empty($related_reviews)) {
    $work->related_reviews = $related_reviews;
  }

  // remove some unnecessary properties
  unset($work->vid);
  unset($work->id);
  unset($work->work_category);
  unset($work->created);
  unset($work->changed);
  return $work;
}
