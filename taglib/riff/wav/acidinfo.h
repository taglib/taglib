/***************************************************************************
 copyright            : (C) 2012 by Rupert Daniel
 email                : rupert@cancelmonday.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_ACIDINFO_H
#define TAGLIB_ACIDINFO_H

#include "taglib_export.h"
#include "taglib.h"

namespace TagLib {
  
  class ByteVector;
  
  namespace RIFF {
    
    namespace WAV {
      
      class File;
      
      //! An implementation of Acid loop information reading for WAV
      
      /*!
       * This reads the Acidised loop data from a chunk in a WAV file.
       */
      
      class TAGLIB_EXPORT AcidInfo
      {
      public:
        
        /*! Flag bits. */
        enum {
          OneShot   = 0x0001,
          RootNote  = 0x0002,
          Stretch   = 0x0004,
          Storage   = 0x0008,
          Unknown   = 0x0010
        };
        
        /*! Root note values. */
        enum {
          RootNoteC         = 0x0030,
          RootNoteCSharp    = 0x0031,
          RootNoteD         = 0x0032,
          RootNoteDSharp    = 0x0033,
          RootNoteE         = 0x0034,
          RootNoteF         = 0x0035,
          RootNoteFSharp    = 0x0036,
          RootNoteG         = 0x0037,
          RootNoteGSharp    = 0x0038,
          RootNoteA         = 0x0039,
          RootNoteASharp    = 0x003A,
          RootNoteB         = 0x003B,
        };

        /*!
         * Create an empty instance of WAV::AcidLoop
         */
        AcidInfo();
        
        /*!
         * Create an instance of WAV::AcidLoop with the data read from the
         * ByteVector \a data.
         */
        AcidInfo(const ByteVector &data);
        
        /*!
         * Destroys this WAV::AcidInfo instance.
         */
        virtual ~AcidInfo();
       
        /*!
         * Returns the Acid loop flags.
         *
         * 0x01 On: One Shot         Off: Loop 
         * 0x02 On: Root note is Set Off: No root 
         * 0x04 On: Stretch is On,   Off: Strech is OFF 
         * 0x08 On: Disk Based       Off: Ram based 
         * 0x10 On: ??????????       Off: ????????? (Acidizer puts that ON) 
         *
         * \see setFlags()
         */ 
        int flags() const;

        /*!
         * Sets the Acid loop flags.
         *
         * \see flags()
         */
        void setFlags(int flags);
       
        /*!
         * Returns the root note of the loop.
         *
         *  [C,C#,(...),B] -> [0x30 to 0x3B] 
         *
         * \see setRootNote()
         */ 
        short rootNote() const;

        /*!
         * Sets the root note of the loop.
         *
         * \see rootNote()
         */
        void setRootNote(short rootNote);
       
        /*!
         * Returns the number of beats in the loop.
         *
         * \see setNumberOfBeats()  
         */
        int numberOfBeats() const;

        /*!
         * Sets the number of beats in the loop.
         *
         * \see numberOfBeats()
         */
        void setNumberOfBeats(int numberOfBeats);
       
        /*!
         * Returns the Denominator component of the time signature.
         *
         * \see setTimeSignatureDenominator()
         */
        short timeSignatureDenominator() const;

        /*!
         * Sets the Denominator component of the time signature.
         *
         * \see timeSignatureDenominator()
         */
        void setTimeSignatureDenominator(short timeSignatureDenominator);
       
        /*!
         * Returns the Numerator component of the time signature.
         *
         * \see setTimeSignatureNumerator()
         */
        short timeSignatureNumerator() const;

        /*!
         * Sets the Numerator component of the time signature.
         *
         * \see timeSignatureNumerator()
         */
        void setTimeSignatureNumerator(short timeSignatureNumerator);
       
        /*!
         * Sets the tempo of the loop.
         *
         * \see setTempo()
         */ 
        float tempo() const;

        /*!
         * Returns the tempo of the loop.
         *
         * \see tempo()
         */
        void setTempo(float tempo);

        /*!
         * Returns true if there is no Acid info set.
         */
        bool isEmpty() const;

        /*!
         * Renders the Acid chunk into a ByteVector ready for writing
         * to the 'acid' chunk in the WAV file.
         */
        ByteVector render() const;
        
      private:
        AcidInfo(const AcidInfo &);
        AcidInfo &operator=(const AcidInfo &);
        
        void read(const ByteVector &data);
        
        class AcidInfoPrivate;
        AcidInfoPrivate *d;
      };
    }
  }
}

#endif

