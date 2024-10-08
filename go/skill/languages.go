// The MIT License (MIT)
//
// Copyright (c) 2024 Xiaoyang Chen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package skill

type Lang int

const (
	UnknownLanguage = Lang(iota)
	English
	Chinese
	Japanese
)

// parse two letter ISO 639 language codes
func ParseLang(name string) (Lang, error) {
	switch name {
	case "zh":
		return Chinese, nil
	case "en":
		return English, nil
	case "ja":
		return Japanese, nil
	default:
		return Lang(-1), ErrInvalidOrUnsupportedLanguageCode
	}
}

func (l Lang) String() string {
	switch l {
	case English:
		return "en"
	case Japanese:
		return "ja"
	case Chinese:
		return "zh"
	default:
		return "unknown"
	}
}
