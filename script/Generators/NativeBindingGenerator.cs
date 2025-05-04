using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Text;

namespace Generators;

[Generator]
public class NativeBindGenerator : IIncrementalGenerator
{
    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        context.RegisterPostInitializationOutput(ctx => 
            ctx.AddSource("NativeBindAttribute.g.cs", """
            using System;
            [AttributeUsage(AttributeTargets.Field, Inherited = false)]
            public sealed class NativeBindAttribute : Attribute
            {
                public string FunctionName { get; }
                public Type ReturnType { get; }
                public Type[] ParameterTypes { get; }
                
                public NativeBindAttribute(string functionName, Type returnType, params Type[] parameterTypes)
                {
                    FunctionName = functionName;
                    ReturnType = returnType;
                    ParameterTypes = parameterTypes;
                }
            }
            """));

        var provider = context.SyntaxProvider
            .CreateSyntaxProvider(
                predicate: static (s, _) => s is FieldDeclarationSyntax fds && 
                    fds.AttributeLists.Count > 0,
                transform: GetFieldBindInfo)
            .Where(static m => m is not null);

        context.RegisterSourceOutput(provider, GenerateCode);
    }

    private static NativeBindInfo? GetFieldBindInfo(
        GeneratorSyntaxContext context, 
        CancellationToken token)
    {
        var fieldSyntax = (FieldDeclarationSyntax)context.Node;
        var attribute = fieldSyntax.AttributeLists
            .SelectMany(a => a.Attributes)
            .FirstOrDefault(a => a.Name.ToString() == "NativeBind");

        if (attribute?.ArgumentList == null) return null;
        
        var arguments = attribute.ArgumentList.Arguments;
        if (arguments.Count < 2) return null;

        var semanticModel = context.SemanticModel;
        
        try
        {
            // Extract function name
            var functionName = semanticModel.GetConstantValue(arguments[0].Expression).ToString();

            // Extract return type
            var returnType = GetTypeName(arguments[1], semanticModel);

            // Extract parameter types
            var paramTypes = new List<string>();
            foreach (var arg in arguments.Skip(2))
            {
                paramTypes.Add(GetTypeName(arg, semanticModel));
            }

            return new NativeBindInfo(
                functionName!,
                returnType!,
                paramTypes.ToArray());
        }
        catch
        {
            return null;
        }
    }

    private static string? GetTypeName(
        AttributeArgumentSyntax arg, 
        SemanticModel semanticModel)
    {
        return (arg.Expression is TypeOfExpressionSyntax typeOfExpr)
            ? semanticModel.GetTypeInfo(typeOfExpr.Type).Type?
                .ToDisplayString(SymbolDisplayFormat.FullyQualifiedFormat)
            : null;
    }

    private static void GenerateCode(
        SourceProductionContext context,
        NativeBindInfo bindInfo)
    {
        var delegateName = $"{bindInfo.FunctionName}Delegate";
        var parameters = string.Join(", ", bindInfo.ParameterTypes.Select((t, i) => $"{t} p{i}"));

        context.AddSource($"{delegateName}.g.cs", $$"""
            using System.Runtime.InteropServices;
            
            namespace Generated
            {
                [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
                public delegate {{bindInfo.ReturnType}} {{delegateName}}({{parameters}});
            }
            """);
    }

    private class NativeBindInfo
    {
        public NativeBindInfo(string functionName, string returnType, string[] parameterTypes)
        {
            FunctionName = functionName;
            ReturnType = returnType;
            ParameterTypes = parameterTypes;
        }
        public string FunctionName;
        public string ReturnType;
        public string[] ParameterTypes;
    }
}

[Generator]
public class DiagnosticGenerator : IIncrementalGenerator
{
    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        context.RegisterPostInitializationOutput(ctx => 
            ctx.AddSource("DiagnosticTest.g.cs", 
                "// Generator is running! 🎉"));
        
        context.RegisterSourceOutput(
            context.CompilationProvider,
            (spc, _) => spc.AddSource("AnotherTest.g.cs",
                "public class GeneratorRanSuccessfully {}"));
    }
}