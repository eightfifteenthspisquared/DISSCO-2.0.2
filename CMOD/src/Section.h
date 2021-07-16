/*
CMOD (composition module)
Copyright (C) 2005  Sever Tipei (s-tipei@uiuc.edu)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SECTION_H
#define SECTION_H

#include "Libraries.h"

#include "TimeSignature.h"
#include "Note.h"

/**
 * A class representing a notated section for output using 
 * Lilypond (or something else in the future).
**/
class Section {

public:
  /**
   * Construct a notation section with the provided time signature.
   * 
   * @param time_signature The time signature of the section
  **/
  Section(TimeSignature time_signature);

  /**
   * Destruct this notation section.
  **/
  ~Section();

  /**
   * Insert a note into this section.
   * 
   * @param n A pointer to the note to insert
   * @return True if the note was inserted; else, false.
  **/
  bool InsertNote(Note* n);

  /**
   * Set the duration of this Section in edus with respect to this
   * Section's time signature. Providing (-1) sets no limit on the 
   * duration.
   * 
   * @param edus The duration of this Section in edus
  **/
  void SetDurationEDUS(int edus);

  /**
   * Get the global start time of this section in seconds.
   * 
   * @return The global start time of this section in seconds
  **/
  float GetStartTimeGlobal() const;

  /**
   * Convert a quantity of seconds to edus with respect to this
   * Section's time signature.
   * 
   * @param seconds The quantity in seconds to convert
   * @return The equivalent edus
  **/
  int CalculateEDUsFromSecondsInTempo(float seconds);
  
  /**
   * Build the text representation of this section by adding bars,
   * rests, and adjusting durations.
  **/
  void Build();

  /**
   * Get this section as a flattened entity ready for output.
  **/
  vector<Note*>& GetSectionFlat();

  bool operator<(const Section& other) const;

  bool operator>(const Section& other) const;

  bool operator==(const Section& other) const;

  bool operator!=(const Section& other) const;

  void PrintAllNotesBar() const {
    size_t bar_idx = 0;
    cout << endl << endl;
    cout << "ALL NOTES BAR: " << endl;
    for (const auto bar : section_) {
      cout << "BAR " << bar_idx << endl;
      for (const auto note : *bar) {
        if (note->type_out.find("\\bar") != string::npos) {
          cout << "BAR    start: " << note->start_t << " end: " << note->end_t << " out: " << note->type_out << endl;
        } else if (note->type_out.find("r") != string::npos) {
          cout << "REST    start: " << note->start_t << " end: " << note->end_t << " out: " << note->type_out << endl;
        } else {
          cout << "NOTE    start: " << note->start_t << " end: " << note->end_t << " pitch: " << note->type_out << endl;
        }
      }
      ++bar_idx;
    }
    cout << endl << endl;
  }

  void PrintAllNotesFlat() const {
    size_t note_idx = 0;
    cout << endl << endl;
    cout << "ALL NOTES FLAT: " << endl;
    for (const auto note : section_flat_) {
      if (note->type_out.find("\\bar") != string::npos) {
        cout << note_idx << " BAR    start: " << note->start_t << " end: " << note->end_t << " out: " << note->type_out << endl;
      } else if (note->type_out.find("r") != string::npos) {
        cout << note_idx << " REST    start: " << note->start_t << " end: " << note->end_t << " out: " << note->type_out << endl;
      } else {
        cout << note_idx << " NOTE    start: " << note->start_t << " end: " << note->end_t << " pitch: " << note->type_out << endl;
      }
      ++note_idx;
    }
  }

private:
  /**
   * If the remainder given by the note's end time modulo the edu's per beat
   * is not divisible by any element of valid_dividers_, find the smallest 
   * difference between a multiple of the valid dividers and the remainder. 
   * Then, shorten the note's duration by this difference to make the note's
   * duration expressible.
   * 
   * @param n A pointer to the note whose duration to examine and modify
  **/
  void EnsureNoteExpressible(Note* n);

  /**
   * Resize the section to the specified size.
   * 
   * @param new_size The desired size of the section
  **/
  void ResizeSection(size_t new_size);

  /**
   * Add bars to this section.
  **/
  void AddBars();

  /**
   * Add rests between notes and flatten the section to section_flat_. 
   * Rests are not processed and may have invalid (inexpressible) 
   * durations.
  **/
  void AddRestsAndFlatten();

  /**
   * Run the notation loop for the section. 
  **/
  void Notate();

  /**
   * Cap the ending of this Section according to the EDU allotment.
  **/
  void CapEnding();

  /**
   * Notate the current note's duration given the previous tuplet type and the current
   * tuplet duration. This involves filling the current tuplet duration, filling the
   * subsequent complete beats if the note extends past the tuplet, and then using
   * rests to fill the remaining duration.
   * 
   * @param current_note The current note whose duration to notate
   * @param prev_tuplet The type of the previous tuplet
   * @param tuplet_dur The duration of the current tuplet to fill
   * @return The remaining duration to carry over in the notation loop
  **/
  int NotateCurrentNote(Note* current_note, int* prev_tuplet, int tuplet_dur);

  /**
   * Fill the current tuplet duration using the current note's duration.
   * 
   * @param current_note The current note whose duration to notate
   * @param prev_tuplet The type of the previous tuplet
   * @param tuplet_dur The duration of the current tuplet to fill
   * @return The remaining note duration
  **/
  int FillCurrentTupletDur(Note* current_note, int prev_tuplet, int tuplet_dur);
  
  /**
   * Fill the complete beats of the remaining current note's duration.
   * 
   * @param current_note The current note whose duration to notate
   * @param remaining_dur The remaining duration of the current note
   * @return The final leftover duration to be notated with rests
  **/
  int FillCompleteBeats(Note* current_note, int remaining_dur);

  /**
   * Create a tuplet with rests for the current note's final remaining duration.
   * 
   * @param current_note The current note whose duration to notate
   * @param prev_tuplet The type of the previous tuplet to set
   * @param remaining_dur The current note's leftover duration to be notated with rests
   * @return The tuplet duration to carry over in the notation loop
  **/
  int CreateTupletWithRests(Note* current_note, int* prev_tuplet, int remaining_dur);

  /**
   * Notate a note (or rest) in a tuplet.
   * 
   * @param current_note A pointer to the note to notate
   * @param tuplet_type The type of the tuplet in which to notate the note
   * @param duration The duration of the note to notate
  **/
  void NoteInTuplet(Note* current_note, int tuplet_type, int duration);

  /**
   * Add a loudness mark to the current note.
   * 
   * @param current_note The note to which to add a loudness mark
  **/
  void LoudnessMark(Note* current_note);

  /**
   * Add a modifier to the current note.
   * 
   * @param current_note The note to which to add a modifier
  **/
  void ModifiersMark(Note* current_note);

  /**
   * Get the first bar of this Section after building.
  **/
  vector<Note*> GetFirstBar();

  /**
   * Get the last bar of this Section after building.
  **/
  list<Note*> GetLastBar();

  TimeSignature time_signature_;

  vector<vector<Note*>*> section_;
  vector<Note*> section_flat_;
  bool is_built_ = false;

  int remaining_edus_ = 0;
  int used_edus_ = 0; // Used edus after building // TODO - move this out of GetLastBar
  bool is_edu_limit_ = true; // true if the Section has an edu allotment; else false

  static string prev_loudness; // the previous loudness mark in notation loop
};

#endif