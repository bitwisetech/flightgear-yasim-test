/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2001                            */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  January 2001                                     */
/*************************************************************************/
/*  Feature functions used by various cart trees etc                     */
/*  These have been create as needed, and as some of the trees are       */
/*  from University of Edinburgh's Festival system their names and       */
/*  semantics follow them                                                */
/*************************************************************************/

/* ----------------------------------------------------------------- */
/*           The English TTS System "Flite+hts_engine"               */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2005-2013  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2005-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "cst_hrg.h"
#include "cst_phoneset.h"
#include "cst_regex.h"
#include "cst_ffeatures.h"
#include "us_ffeatures.h"

static const cst_val *gpos(const cst_item *word);

DEF_STATIC_CONST_VAL_STRING(val_string_numeric,"numeric");
DEF_STATIC_CONST_VAL_STRING(val_string_number,"number");
DEF_STATIC_CONST_VAL_STRING(val_string_month,"month");
DEF_STATIC_CONST_VAL_STRING(val_string_day,"day");
DEF_STATIC_CONST_VAL_STRING(val_string_other,"_other_");
DEF_STATIC_CONST_VAL_STRING(val_string_a,"a");
DEF_STATIC_CONST_VAL_STRING(val_string_flight,"flight");
DEF_STATIC_CONST_VAL_STRING(val_string_to,"to");

DEF_STATIC_CONST_VAL_STRING(val_string_content,"content");

static const cst_val *gpos(const cst_item *word)
{
    /* Guess at part of speech (function/content) */
    const char *w;
    int s,t;

    w = item_feat_string(word,"name");

    for (s=0; us_gpos[s]; s++)
    {
	for (t=1; us_gpos[s][t]; t++)
	    if (cst_streq(w,val_string(us_gpos[s][t])))
		return us_gpos[s][0];
    }

    return (cst_val *)&val_string_content;
}

static const cst_val *num_digits(const cst_item *token)
{   
    const char *name = item_feat_string(token,"name");

    return val_int_n(cst_strlen(name));
}

static const cst_val *month_range(const cst_item *token)
{   
    int v = item_feat_int(token,"name");

    if ((v > 0) && ( v < 32))
	return VAL_STRING_1;	
    else
	return VAL_STRING_0;	
}

static const cst_val* token_pos_guess(const cst_item* token)
{
    const char* name = item_feat_string(token, "name");
    char* dc = cst_downcase(name);
    const cst_val* r;

    if (cst_regex_match(cst_rx_digits, dc))
        r = (cst_val*)&val_string_numeric;
    else if ((cst_regex_match(cst_rx_double, dc)) ||
             (cst_regex_match(cst_rx_double, dc)))
        r = (cst_val*)&val_string_number;
    else if (cst_streq(dc, "jan") ||
             cst_streq(dc, "january") ||
             cst_streq(dc, "feb") ||
             cst_streq(dc, "february") ||
             cst_streq(dc, "mar") ||
             cst_streq(dc, "march") ||
             cst_streq(dc, "apr") ||
             cst_streq(dc, "april") ||
             cst_streq(dc, "may") ||
             cst_streq(dc, "jun") ||
             cst_streq(dc, "june") ||
             cst_streq(dc, "jul") ||
             cst_streq(dc, "july") ||
             cst_streq(dc, "aug") ||
             cst_streq(dc, "august") ||
             cst_streq(dc, "sep") ||
             cst_streq(dc, "sept") ||
             cst_streq(dc, "september") ||
             cst_streq(dc, "oct") ||
             cst_streq(dc, "october") ||
             cst_streq(dc, "nov") ||
             cst_streq(dc, "november") ||
             cst_streq(dc, "dec") ||
             cst_streq(dc, "december"))
        r = (cst_val*)&val_string_month;
    else if (cst_streq(dc, "sun") ||
             cst_streq(dc, "sunday") ||
             cst_streq(dc, "mon") ||
             cst_streq(dc, "monday") ||
             cst_streq(dc, "tue") ||
             cst_streq(dc, "tues") ||
             cst_streq(dc, "tuesday") ||
             cst_streq(dc, "wed") ||
             cst_streq(dc, "wednesday") ||
             cst_streq(dc, "thu") ||
             cst_streq(dc, "thurs") ||
             cst_streq(dc, "thursday") ||
             cst_streq(dc, "fri") ||
             cst_streq(dc, "friday") ||
             cst_streq(dc, "sat") ||
             cst_streq(dc, "saturday"))
        r = (cst_val*)&val_string_day;
    /* ignoring the "token_most_common" condition, does get used */
    else if (cst_streq(dc, "a"))
        r = (cst_val*)&val_string_a;
    else if (cst_streq(dc, "flight"))
        r = (cst_val*)&val_string_flight;
    else if (cst_streq(dc, "to"))
        r = (cst_val*)&val_string_to;
    else
        r = (cst_val*)&val_string_other;

    cst_free(dc);

    return r;
}

#ifdef FLITE_PLUS_HTS_ENGINE

const cst_val *accented(const cst_item *p); /* defined in cst_ffeatures.c */

int item_after_length(const cst_item *n){
    int i = 0;
    
    if(n == NULL) return 0;
    
    for(;n;n=n->n,i++);
    
    return i;
}

const cst_item *item_first(const cst_item *n){
  if(n == 0) return 0;
  for(;n->p != 0;n=n->p);
  return n;
}

/* 11 12 13 53 54 */
static const cst_val *syl_numphones(const cst_item *syl){
  return val_string_n(item_after_length(item_daughter(item_as(syl,"SylStructure"))));
}

/* 14 */
static const cst_val *pos_in_word(const cst_item *syl){
  const cst_item *ss,*p;
  int c = 0;
    
  ss = item_as(syl,"SylStructure");
  for(p = item_daughter(item_parent(ss));p;p = item_next(p),c++){
    if(p == ss) return val_string_n(c);
  }
  return val_string_n(c);
}

/* 21 by Toda-san */
static const cst_val *lisp_distance_to_p_stress(const cst_item *syl){
  const cst_item *s, *fs;
  int c;
  
  s=item_as(syl,"Syllable");
  fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");
  if (item_equal(s,fs)) return val_string_n(0);
  s=item_prev(s);
  for (c=1; s && (!item_equal(s,fs)) && (c < CST_CONST_INT_MAX);
       s=item_prev(s),c++)
    if (strcmp("1", ffeature_string(s,"stress")) == 0) return val_string_n(c);
  if (strcmp("1", ffeature_string(s,"stress")) == 0) return val_string_n(c);
  else return val_string_n(0);
}

/* 22 by Toda-san */
static const cst_val *lisp_distance_to_n_stress(const cst_item *syl){
  const cst_item *s, *fs;
  int c;
  
  s=item_as(syl,"Syllable");
  fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughtern.R:SylStructure.daughtern");
  if (item_equal(s,fs)) return val_string_n(0);
  s=item_next(s);
  for (c=1; s && (!item_equal(s,fs)) && (c < CST_CONST_INT_MAX);
       s=item_next(s),c++)
    if (strcmp("1", ffeature_string(s,"stress")) == 0) return val_string_n(c);
  if (strcmp("1", ffeature_string(s,"stress")) == 0) return val_string_n(c);
  else return val_string_n(0);
}

/* 23 by Toda-san */
static const cst_val *lisp_distance_to_p_accent(const cst_item *syl){
  const cst_item *s, *fs;
  int c;
  
  s=item_as(syl,"Syllable");
  fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughter.R:SylStructure.daughter");
  if (item_equal(s,fs)) return val_string_n(0);
  s=item_prev(s);
  for (c=1; s && (!item_equal(s,fs)) && (c < CST_CONST_INT_MAX);
       s=item_prev(s),c++)
    if (val_int(accented(s))) return val_string_n(c);
  if (val_int(accented(s))) return val_string_n(c);
  else return val_string_n(0);
}

/* 24 by Toda-san */
static const cst_val *lisp_distance_to_n_accent(const cst_item *syl){
  const cst_item *s, *fs;
  int c;
  
  s=item_as(syl,"Syllable");
  fs = path_to_item(syl,"R:SylStructure.parent.R:Phrase.parent.daughtern.R:SylStructure.daughtern");
  if (item_equal(s,fs)) return val_string_n(0);
  s=item_next(s);
  for (c=1; s && (!item_equal(s,fs)) && (c < CST_CONST_INT_MAX);
       s=item_next(s),c++)
    if (val_int(accented(s))) return val_string_n(c);
  if (val_int(accented(s))) return val_string_n(c);
  else return val_string_n(0);
}

/* 25 */
static char syl_vowel_str[16];
static cst_val syl_vowel_val;
static const cst_val *syl_vowel(const cst_item *syl){
  /* the vowel in the syllable */
  const cst_item *ss,*p;
  int size;
  
  CST_VAL_TYPE(&syl_vowel_val) = CST_VAL_TYPE_STRING;
  ss = item_as(syl,"SylStructure");
  for(p = item_daughter(ss),size = 0;p;p = item_next(p),size++){
    if(strchr("aeiou",item_name(p)[0]) != NULL){
      strcpy(syl_vowel_str,item_name(p));
      CST_VAL_STRING_LVAL(&syl_vowel_val) = syl_vowel_str;
      return &syl_vowel_val;
    }
  }
  /* no vowel */
  strcpy(syl_vowel_str,"novowel");
  CST_VAL_STRING_LVAL(&syl_vowel_val) = syl_vowel_str;
  return &syl_vowel_val;
}

/* 32 */
static const cst_val *pos_in_phrase(const cst_item *syl){
  const cst_item *ss,*p;
  int c = 0;
  
  ss = item_as(syl,"Phrase");
  for (p = item_first(ss);p;p=item_next(p),c++)
    if(p == ss) return val_string_n(c);
  return val_string_n(c);
}

/* 33 */
static const cst_val *words_out(const cst_item *syl){
  const cst_item *ss,*p;
  int c = 0;
  
  ss = item_as(syl,"Phrase");
  for(p = ss;p;p=item_next(p),c++);
  return val_string_n(c);
}

/* 34 by Toda-san */
static const cst_val *content_words_in(const cst_item *word){
  const cst_item *ss,*p,*fs;
  int c;
  
  ss = word;
  fs = path_to_item(word,"R:Phrase.parent.daughter");
  
  for (c=0, p=ss; p && (!item_equal(p,fs)) && (c < CST_CONST_INT_MAX);
	 p=item_prev(p))
    if (cst_streq("content", ffeature_string(p,"gpos"))) c++;
  
  return val_string_n(c);  /* its used randomly as int and float */
}

/* 35 by Toda-san */
static const cst_val *content_words_out(const cst_item *word){
  const cst_item *ss,*p,*fs;
  int c;
  
  ss = word;
  fs = path_to_item(word,"R:Phrase.parent.daughtern");
  
  for (c=0, p=ss; p && (!item_equal(p,fs)) && (c < CST_CONST_INT_MAX); 
       p=item_next(p))
    if (cst_streq("content", ffeature_string(p,"gpos"))) c++;
  
  return val_string_n(c);  /* its used randomly as int and float */
}

/* 36 */
static const cst_val *lisp_distance_to_p_content(const cst_item *syl){
  const cst_item *p;
  int c = 0;

  for(p=item_prev(item_as(syl,"Phrase"));p;p=item_prev(p)){
    c++;
    if(gpos(p)==(cst_val*)&val_string_content)
      break;
  }
  return val_string_n(c);
}

/* 37 */
static const cst_val *lisp_distance_to_n_content(const cst_item *syl){
  const cst_item *p;
  int c = 0;

  for(p=item_next(item_as(syl,"Phrase"));p;p=item_next(p)){
    c++;
    if (gpos(p)==(cst_val*)&val_string_content)
      break;
  }
  return val_string_n(c);
}

/* 38 39 40 59 60 by Toda-san */
static const cst_val *lisp_num_syls_in_phrase(const cst_item *phrase){
  const cst_item *sw,*fw;
  int c;
  
  sw = path_to_item(phrase,"daughter");
  fw = path_to_item(phrase,"daughtern");
  
  for (c=0; sw && (!item_equal(sw,fw)) && (c < CST_CONST_INT_MAX);
       sw=item_next(sw)) c += ffeature_int(sw, "word_numsyls");
  c += ffeature_int(sw, "word_numsyls");
  
  return val_string_n(c);
}

/* 41 42 43 61 62 by Toda-san */
static const cst_val *lisp_num_words_in_phrase(const cst_item *phrase){
  const cst_item *sw,*fw;
  int c;
  
  sw = path_to_item(phrase,"daughter");
  fw = path_to_item(phrase,"daughtern");
  
  for (c=1; sw && (!item_equal(sw,fw)) && (c < CST_CONST_INT_MAX);
       sw=item_next(sw)) c++;
  
  return val_string_n(c);
}

/* 46 by Toda-san */
static const cst_val *lisp_total_syls(const cst_item *phrase){
  const cst_item *sp, *fp;
  int c;
  
  sp = phrase;
  while (item_prev(sp) != NULL) sp = item_prev(sp);
  fp = phrase;
  while (item_next(fp) != NULL) fp = item_next(fp);
  
  for (c = 0; sp && (!item_equal(sp, fp)) && (c < CST_CONST_INT_MAX);
       sp = item_next(sp)) c += ffeature_int(sp, "lisp_num_syls_in_phrase");
  c += ffeature_int(sp, "lisp_num_syls_in_phrase");
  return val_string_n(c);
}

/* 47 by Toda-san */
static const cst_val *lisp_total_words(const cst_item *phrase){
  const cst_item *sp, *fp;
  int c;
  
  sp = phrase;
  while (item_prev(sp) != NULL) sp = item_prev(sp);
  fp = phrase;
  while (item_next(fp) != NULL) fp = item_next(fp);
  
  for (c = 0; sp && (!item_equal(sp, fp)) && (c < CST_CONST_INT_MAX);
       sp = item_next(sp)) c += ffeature_int(sp, "lisp_num_words_in_phrase");
  c += ffeature_int(sp, "lisp_num_words_in_phrase");
  return val_string_n(c);
}

/* 48 by Toda-san */
static const cst_val *lisp_total_phrases(const cst_item *phrase){
  const cst_item *sp, *fp;
  int c;
  
  sp = phrase;
  while (item_prev(sp) != NULL) sp = item_prev(sp);
  fp = phrase;
  while (item_next(fp) != NULL) fp = item_next(fp);
  
  for (c = 1; sp && (!item_equal(sp, fp)) && (c < CST_CONST_INT_MAX);
       sp = item_next(sp)) c++;
  
  return val_string_n(c);
}

#endif /* FLITE_PLUS_HTS_ENGINE */

void us_ff_register(cst_features *ffunctions)
{

    /* The language independent ones */
    basic_ff_register(ffunctions);

    ff_register(ffunctions, "gpos",gpos);
    ff_register(ffunctions, "num_digits",num_digits);
    ff_register(ffunctions, "month_range",month_range);
    ff_register(ffunctions, "token_pos_guess",token_pos_guess);

#ifdef FLITE_PLUS_HTS_ENGINE
    ff_register(ffunctions, "syl_numphones",syl_numphones); /* 11 12 13 53 54 */
    ff_register(ffunctions, "pos_in_word",pos_in_word); /* 14 */
    ff_register(ffunctions, "lisp_distance_to_p_stress",lisp_distance_to_p_stress); /* 21 */
    ff_register(ffunctions, "lisp_distance_to_n_stress",lisp_distance_to_n_stress); /* 22 */
    ff_register(ffunctions, "lisp_distance_to_p_accent",lisp_distance_to_p_accent); /* 23 */
    ff_register(ffunctions, "lisp_distance_to_n_accent",lisp_distance_to_n_accent); /* 24 */
    ff_register(ffunctions, "syl_vowel",syl_vowel); /* 25 */
    ff_register(ffunctions, "pos_in_phrase",pos_in_phrase); /* 32 */
    ff_register(ffunctions, "words_out",words_out); /* 33 */
    ff_register(ffunctions, "content_words_in",content_words_in); /* 34 */
    ff_register(ffunctions, "content_words_out",content_words_out); /* 35 */
    ff_register(ffunctions, "lisp_distance_to_p_content",lisp_distance_to_p_content); /* 36 */
    ff_register(ffunctions, "lisp_distance_to_n_content",lisp_distance_to_n_content); /* 37 */
    ff_register(ffunctions, "lisp_num_syls_in_phrase",lisp_num_syls_in_phrase); /* 38 39 40 59 60 */
    ff_register(ffunctions, "lisp_num_words_in_phrase",lisp_num_words_in_phrase); /* 41 42 43 61 62 */
    ff_register(ffunctions, "lisp_total_syls",lisp_total_syls); /* 46 */
    ff_register(ffunctions, "lisp_total_words",lisp_total_words); /* 47 */
    ff_register(ffunctions, "lisp_total_phrases",lisp_total_phrases); /* 48 */
#endif /* FLITE_PLUS_HTS_ENGINE */
}
