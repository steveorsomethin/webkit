/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.AnalyzerMessage = function(sourceCodeLocation, text, ruleIdentifier)
{
    WebInspector.Object.call(this);

    console.assert(sourceCodeLocation instanceof WebInspector.SourceCodeLocation);
    console.assert(typeof text === "string");

    this._sourceCodeLocation = sourceCodeLocation;
    this._text = text;
    this._ruleIdentifier = ruleIdentifier;
};

WebInspector.AnalyzerMessage.prototype = {
    constructor: WebInspector.AnalyzerMessage,
    __proto__: WebInspector.Object.prototype,

    get sourceCodeLocation()
    {
        return this._sourceCodeLocation;
    },

    get sourceCode()
    {
        return this._sourceCodeLocation.sourceCode;
    },

    get text()
    {
        return this._text;
    },

    get ruleIdentifier()
    {
        return this._ruleIdentifier;
    }
};
